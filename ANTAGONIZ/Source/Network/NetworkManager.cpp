#include "NetworkManager.hpp"
#include "Debug.hpp"
#include <chrono>
using namespace Ge;

NetworkManager::NetworkManager(Role role, const std::string& ip, int port) : role(role), server_ip(ip), server_port(port)
{
   registerFunction<int>("SetClientID", [=](int cID)
   {
        idClient = cID;
        Debug::Log("Set client ID : %d", idClient);
   });
}

bool NetworkManager::start() 
{
    if (role == Role::SERVER || role == Role::BOTH)
    {
        return startServer();
    }
    else 
    {
        return startClient();
    }
}

void NetworkManager::stop() 
{
    onClientDisconnect.clear();
    onClientConnect.clear();
    if (role == Role::SERVER || role == Role::BOTH)
    {
        if (role == Role::BOTH)
        {
            client.stop();
        }
        server.stop();
    }
    else 
    {
        client.stop();
    }
}

void NetworkManager::update()
{
    if (isServer())
    {
        if (m_tocalls.size() > 0)
        {
            std::lock_guard<std::mutex> lock(mtx_call);
            for (auto& call : m_tocalls)
            {
                call.second(call.first);
            }
            m_tocalls.clear();
        }
    }
    if (isClient())
    {
        if(disconnectFromServeur)
        {
            for (int i = 0; i < onServeurDisconnectClient.size(); i++)
            {
                onServeurDisconnectClient[i]();
            }
            onServeurDisconnectClient.clear();
            disconnectFromServeur = false;
        }
    }
    if(exectutors.size() > 0)
    {
        std::lock_guard<std::mutex> lock(mtx_executor);
        for (int i = 0; i < exectutors.size(); i++)
        {
            executeSerializedCall(exectutors[i]);
        }
        exectutors.clear();
    }
}

bool NetworkManager::startServer() 
{
    int listenfd = server.createsocket(server_port, "0.0.0.0");
    if (listenfd < 0) 
    {
        Debug::Error("Failed to start server");
        return false;
    }
    Debug::Log("server listen on port %d, listenfd=%d ...\n", server_port, listenfd);
    server.onConnection = [&](const hv::SocketChannelPtr& channel) 
    {
        if (channel->isConnected()) 
        {
            int clientId = nextClientId++;
            clients[clientId] = channel;           
            {
                std::lock_guard<std::mutex> lock(mtx_call);
                m_tocalls.push_back({ clientId,[=](int cid)
                {                        
                        //call(MessageType::CLIENT_ONLY_TARGET, cid, "SetClientID", cid);
                        call(MessageType::CLIENT_ONLY_TARGET, cid, "SetClientID", cid);
                } });
                for (const auto& ocd : onClientConnect) { m_tocalls.push_back({ clientId, ocd.second }); }
                
            }
            Debug::Log("[SERVER] Client %d connecte.",clientId);
        }
        else
        {
            for (auto it = clients.begin(); it != clients.end();) 
            {
                if (it->second == channel) 
                {
                    {
                        std::lock_guard<std::mutex> lock(mtx_call);
                        for (const auto& ocd : onClientDisconnect[it->first]) { m_tocalls.push_back({ it->first,ocd.second }); }
                    }
                    Debug::Log("[SERVER] Client %d deconnecte.", it->first);                    
                    it = clients.erase(it);      
                    break;
                }
                else 
                {
                    ++it;
                }
            }            
        }
    };

    server.onMessage = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) 
    {
        auto& recvBuffer = recvBufferServer[channel]; // buffer associé à ce client
        recvBuffer.insert(recvBuffer.end(),
            (uint8_t*)buf->data(),
            (uint8_t*)buf->data() + buf->size());

        while (true)
        {
            if (recvBuffer.size() < sizeof(uint32_t))
            {
                return;
            }

            uint32_t size = 0;
            std::memcpy(&size, recvBuffer.data(), sizeof(uint32_t));

            if (recvBuffer.size() < sizeof(uint32_t) + size)
            {
                return;
            }

            FunctionRegistry::RawData msg(recvBuffer.begin() + sizeof(uint32_t), recvBuffer.begin() + sizeof(uint32_t) + size);
            FunctionRegistry::RawData msgComplet(recvBuffer.begin(), recvBuffer.begin() + sizeof(uint32_t) + size);
            const char* p = (char*)msg.data();
            MessageType type;
            int32_t targetClient = -1;
            std::memcpy(&type, p, sizeof(type));
            switch (type)
            {
            case MessageType::SERVER_ONLY:
                {
                    std::lock_guard<std::mutex> lock(mtx_executor);
                    exectutors.push_back(msg);
                }
                break;
            case MessageType::SERVER_CLIENT:
                {
                    std::lock_guard<std::mutex> lock(mtx_executor);
                    exectutors.push_back(msg);
                }
                broadcast(channel, msgComplet);
                break;
            case MessageType::CLIENT_ONLY:
                broadcast(channel, msgComplet);
                break;
            case MessageType::CLIENT_ONLY_TARGET:
                p += sizeof(type);
                std::memcpy(&targetClient, p, sizeof(targetClient));
                sendToClient(targetClient, msg);
                break;
            }

            // Supprimer du buffer ce qui a été traité
            recvBuffer.erase(
                recvBuffer.begin(),
                recvBuffer.begin() + sizeof(uint32_t) + size
            );
        }
    };   
    
    server.max_connections = 999;
    server.host = "0.0.0.0";
    server.setThreadNum(4);
    server.start();
    if (role == Role::BOTH)
    {
        return startClient();
    }
    return true;
}

bool NetworkManager::startClient() 
{
    int connfd = client.createsocket(server_port, server_ip.c_str());
    if (connfd < 0) 
    {
        Debug::Error("Failed to start client");
        return false;
    }

    client.onConnection = [&](const hv::SocketChannelPtr& channel) 
    {
        if (channel->isConnected()) 
        {
            Debug::Log("[CLIENT] Connecte au serveur");                 
            connectedToServeur = true;
        }
        else 
        {
            Debug::Log("[CLIENT] Deconnecte du serveur");
            disconnectFromServeur = true;
        }
    };

    client.onMessage = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) 
    {
        recvBufferClient.insert(recvBufferClient.end(),(uint8_t*)buf->data(),(uint8_t*)buf->data() + buf->size());

        while (true)
        {
            if (recvBufferClient.size() < sizeof(uint32_t))
            {
                return;
            }

            uint32_t size = 0;
            std::memcpy(&size, recvBufferClient.data(), sizeof(uint32_t));
            if (recvBufferClient.size() < sizeof(uint32_t) + size)
            {
                return;
            }

            FunctionRegistry::RawData msg(
                recvBufferClient.begin() + sizeof(uint32_t),
                recvBufferClient.begin() + sizeof(uint32_t) + size
            );
                       
            {
                std::lock_guard<std::mutex> lock(mtx_executor);
                exectutors.push_back(msg);                
            }

            recvBufferClient.erase(
                recvBufferClient.begin(),
                recvBufferClient.begin() + sizeof(uint32_t) + size
            );
        } 
    };    
    int timeout_ms = 3000;
    client.setConnectTimeout(timeout_ms);  // 5 secondes       
    client.start();

    int waited = 0;
    const int interval = 50; // ms
    while (!connectedToServeur && waited < timeout_ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        waited += interval;
    }

    if (!connectedToServeur)
    {
        Debug::Warn("Timeout : impossible de se connecter au serveur");
        client.closesocket();
        return false;
    }

    if (client.isConnected())
    {
        auto start = std::chrono::steady_clock::now();
        while (true) 
        {
            update();
            if (idClient != -1)
            {
                return true;
            }

            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= 5.0f) 
            {
                return false; // timeout atteint
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }    
    Debug::Warn("Gros fail client eclatax");
    return false;
}

void NetworkManager::executeSerializedCall(const FunctionRegistry::RawData & msg)
{
    const uint8_t * p = msg.data();
    MessageType type;
    int32_t targetClient = -1;
    uint32_t hashName;

    std::memcpy(&type, p, sizeof(type));
    p += sizeof(type);
    std::memcpy(&targetClient, p, sizeof(targetClient));
    p += sizeof(targetClient);
    std::memcpy(&hashName, p, sizeof(hashName));
    p += sizeof(hashName);

    FunctionRegistry::RawData args(p, msg.data() + msg.size());
    registry.receive(hashName, args);
}

void NetworkManager::sendToServer(const FunctionRegistry::RawData& data)
{
    if (client.isConnected())
    {
        client.send(data.data(), data.size());
    }
}

void NetworkManager::sendToClient(int id, const FunctionRegistry::RawData& data)
{
    auto it = clients.find(id);
    if (it != clients.end() && it->second->isConnected()) 
    {
        it->second->write(data.data(), data.size());
    }
}

void NetworkManager::broadcast(const FunctionRegistry::RawData& data)
{
    for (auto& [id, channel] : clients) 
    {
        if (channel->isConnected()) 
        {
            channel->write(data.data(), data.size());
        }
    }
}

void NetworkManager::broadcast(const hv::SocketChannelPtr& ignoreClientID,const FunctionRegistry::RawData& data)
{
    for (auto& [id, channel] : clients)
    {
        if (channel->isConnected() && channel != ignoreClientID)
        {
            channel->write(data.data(), data.size());
        }
    }
}
