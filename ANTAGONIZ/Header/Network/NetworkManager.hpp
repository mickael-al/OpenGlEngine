#ifndef __NETWORK_MANAGER__
#define __NETWORK_MANAGER__

#define HV_DLL
#include <string>
#include <unordered_map>
#include <functional>
#include <TcpServer.h>
#include <TcpClient.h>
#include "FunctionRegistry.hpp"
#include <cstdint>
#include <string>

class HashUtil 
{
public:
    static constexpr uint32_t DEFAULT_SEED32 = 0x811C9DC5u;
    static constexpr uint64_t DEFAULT_SEED64 = 0xcbf29ce484222325ULL;

    // Hash 32 bits
    static uint32_t hash32(const std::string& str, uint32_t seed = DEFAULT_SEED32) 
    {
        uint32_t hash = seed;
        for (unsigned char c : str) 
        {
            hash ^= c;
            hash *= 16777619u; // FNV prime 32
        }
        return hash;
    }

    // Hash 64 bits
    static uint64_t hash64(const std::string& str, uint64_t seed = DEFAULT_SEED64) 
    {
        uint64_t hash = seed;
        for (unsigned char c : str) 
        {
            hash ^= c;
            hash *= 1099511628211ULL; // FNV prime 64
        }
        return hash;
    }
};

enum class Role { SERVER, CLIENT, BOTH };
enum class MessageType {
    SERVER_ONLY,
    SERVER_CLIENT,
    CLIENT_ONLY,
    CLIENT_ONLY_TARGET,
    BRODCAST,
};

class NetworkManager 
{
public:
    using FunctionWrapper = std::function<void(const std::vector<std::string>&)>;   

    NetworkManager(Role role, const std::string& ip, int port);
    bool start();
    void stop();
    bool isServer() { return role == Role::SERVER || Role::BOTH == role; }
    bool isClient() { return role == Role::CLIENT || Role::BOTH == role; }
    void update();
    int getClientId() const { return idClient; } // uniquement pour les clients
    template<typename... Args>
    void registerFunction(const std::string& name, std::function<void(Args...)> fn) 
    {
        registry.registerFunction(HashUtil::hash32(name), fn);
    }

    template<typename... Args>
    void call(MessageType type, int targetClientId, const std::string& name, Args... args)
    {
        uint32_t hashName = HashUtil::hash32(name);
        FunctionRegistry::RawData argsData = registry.extract(hashName, args...);
        FunctionRegistry::RawData message;
        message.insert(message.end(), (uint8_t*)&type, (uint8_t*)&type + sizeof(type));
        message.insert(message.end(), (uint8_t*)&targetClientId, (uint8_t*)&targetClientId + sizeof(targetClientId));
        message.insert(message.end(), (uint8_t*)&hashName, (uint8_t*)&hashName + sizeof(hashName));
        message.insert(message.end(), argsData.begin(), argsData.end());
        uint32_t len = (uint32_t)message.size();
        FunctionRegistry::RawData finalMsg;
        finalMsg.insert(finalMsg.end(), (uint8_t*)&len, (uint8_t*)&len + sizeof(len));
        finalMsg.insert(finalMsg.end(), message.begin(), message.end());
        switch (type) 
        {
        case MessageType::SERVER_ONLY:
            if (isServer())
            {
                executeSerializedCall(message);
            }
            else
            {
                sendToServer(finalMsg);
            }
            break;
        case MessageType::SERVER_CLIENT:
            if (isServer())
            {
                executeSerializedCall(message);
                broadcast(finalMsg);
            }
            else
            {
                sendToServer(finalMsg);
            }
            break;
        case MessageType::CLIENT_ONLY:
            if (Role::SERVER == role)
            {
                broadcast(finalMsg);
            }
            else
            {
                sendToServer(finalMsg);
            }
            break;
        case MessageType::CLIENT_ONLY_TARGET:
            if (isServer())
            {
                sendToClient(targetClientId, finalMsg);
            }
            else
            {
                sendToServer(finalMsg);
            }
            break;
        case MessageType::BRODCAST:
            if (isServer())
            {
                broadcast(finalMsg);
            }
            break;
        }
    }

    template<typename... Args>
    void call(MessageType type, const std::string& name, Args... args)
    {
        call(type, -1, name, args...);
    }

    template<typename... Args>
    void execf(const std::string& name, Args... args)
    {
        uint32_t hashName = HashUtil::hash32(name);
        FunctionRegistry::RawData argsData = registry.extract(hashName, args...);
        registry.receive(hashName, argsData);
    }

    void addClientConnectFunction(std::string name, std::function<void(int)> fn)
    {
        onClientConnect[name] = fn;
    }

    void addClientDisconnectFunction(int cid, std::string name, std::function<void(int)> fn)
    {
        onClientDisconnect[cid][name] = fn;
    }

    void removeClientConnectFunction(std::string name)
    {
        auto it = onClientConnect.find(name);
        if (it != onClientConnect.end()) 
        {
            onClientConnect.erase(it);
        }
    }

    void removeClientDisconnectFunction(int cid,std::string name)
    {
        auto it = onClientDisconnect.find(cid);
        if (it != onClientDisconnect.end())
        {
            auto itn = it->second.find(name);
            if (itn != it->second.end())
            {
                it->second.erase(itn);
            }
        }
    }
    std::vector< std::function<void()>>& getOnServeurDisconnectClient() { return onServeurDisconnectClient; }
private:
    bool startClient();
    bool startServer();
    void executeSerializedCall(const FunctionRegistry::RawData& data);
    void sendToServer(const FunctionRegistry::RawData& data);
    void sendToClient(int id, const FunctionRegistry::RawData& data);
    void broadcast(const FunctionRegistry::RawData& data);
    void broadcast(const hv::SocketChannelPtr& ignoreClientID,const FunctionRegistry::RawData& data);
private:
    Role role;
    std::string server_ip;
    int server_port;
    
    // --- Serveur ---
    hv::TcpServer server;
    std::unordered_map<int, hv::SocketChannelPtr> clients;
    std::unordered_map<hv::SocketChannelPtr, FunctionRegistry::RawData> recvBufferServer;
    std::map<std::string, std::function<void(int)>> onClientConnect;
    std::map<int, std::map<std::string, std::function<void(int)>>> onClientDisconnect;
    std::mutex mtx_call;
    std::vector<std::pair<int, std::function<void(int)>>> m_tocalls;
    int nextClientId = 1;

    // --- Client ---
    hv::TcpClient client;    
    FunctionRegistry::RawData recvBufferClient;
    int idClient = -1; // id du client côté client
    std::vector< std::function<void()>> onServeurDisconnectClient;
    bool disconnectFromServeur = false;
    bool connectedToServeur = false;

    // --- function ---
    FunctionRegistry registry;
    std::mutex mtx_executor;
    std::vector<FunctionRegistry::RawData> exectutors;

};
#endif //!__NETWORK_MANAGER__