#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <any>
#include <tuple>
#include <cstring>
#include <stdexcept>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

class FunctionRegistry 
{
public:
    using RawData = std::vector<uint8_t>;

    struct IFunctor {
        virtual ~IFunctor() = default;
        virtual void callFromBinary(const RawData& data) = 0;
    };

    template<typename... Args>
    struct Functor : IFunctor {
        std::function<void(Args...)> fn;

        Functor(std::function<void(Args...)> f) : fn(std::move(f)) {}

        void callFromBinary(const RawData& data) override {
            auto tup = deserializeTuple<Args...>(data);
            std::apply(fn, tup);
        }
    };

    std::map<uint32_t, std::unique_ptr<IFunctor>> functions;

public:
    template<typename... Args>
    void registerFunction(uint32_t name, std::function<void(Args...)> fn) {
        functions[name] = std::make_unique<Functor<Args...>>(std::move(fn));
    }

    template<typename... Args>
    RawData extract(uint32_t name, Args... args) {
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw std::runtime_error("Function not found: " + name);
        }

        // sérialise les arguments
        RawData data = serializeTuple(std::make_tuple(args...));

        return data;
    }

    void receive(uint32_t name, const RawData& data) {
        auto it = functions.find(name);
        if (it == functions.end()) 
        {
            throw std::runtime_error("Function not found: " + name);            
        }
        it->second->callFromBinary(data);
    }

private:
    template<typename T>
    static void serializeOne(RawData& buf, const T& value) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
        buf.insert(buf.end(), p, p + sizeof(T));
    }

    static void serializeOne(RawData& buf, const std::string& value) 
    {
        uint32_t len = static_cast<uint32_t>(value.size());
        serializeOne(buf, len); // On écrit la taille de la chaîne
        buf.insert(buf.end(), value.begin(), value.end()); // Puis le contenu
    }

    static void serializeOne(RawData& buf, const glm::vec3& value)
    {
        serializeOne(buf, value.x);
        serializeOne(buf, value.y);
        serializeOne(buf, value.z);        
    }

    static void serializeOne(RawData& buf, const glm::quat& value)
    {
        serializeOne(buf, value.x);
        serializeOne(buf, value.y);
        serializeOne(buf, value.z);
        serializeOne(buf, value.w);
    }

    template<typename Tuple, size_t... I>
    static RawData serializeTupleImpl(const Tuple& t, std::index_sequence<I...>) 
    {
        RawData buf;
        (serializeOne(buf, std::get<I>(t)), ...);
        return buf;
    }

    template<typename... Args>
    static RawData serializeTuple(const std::tuple<Args...>& t) 
    {
        return serializeTupleImpl(t, std::index_sequence_for<Args...>{});
    }

    template<typename T>
    static T deserializeOne(const uint8_t*& p) 
    {
        T val;
        std::memcpy(&val, p, sizeof(T));
        p += sizeof(T);
        return val;
    }

    template<>
    static std::string deserializeOne<std::string>(const uint8_t*& p) 
    {
        uint32_t len;
        std::memcpy(&len, p, sizeof(uint32_t));
        p += sizeof(uint32_t);
        std::string s(reinterpret_cast<const char*>(p), len);
        p += len;
        return s;
    }

    template<>
    static glm::vec3 deserializeOne<glm::vec3>(const uint8_t*& p)
    {
        glm::vec3 pos;
        std::memcpy(&pos.x, p, sizeof(float)); p += sizeof(float);
        std::memcpy(&pos.y, p, sizeof(float)); p += sizeof(float);
        std::memcpy(&pos.z, p, sizeof(float)); p += sizeof(float);
        return pos;
    }

    template<>
    static glm::quat deserializeOne<glm::quat>(const uint8_t*& p)
    {
        glm::quat rot;
        std::memcpy(&rot.x, p, sizeof(float)); p += sizeof(float);
        std::memcpy(&rot.y, p, sizeof(float)); p += sizeof(float);
        std::memcpy(&rot.z, p, sizeof(float)); p += sizeof(float);
        std::memcpy(&rot.w, p, sizeof(float)); p += sizeof(float);
        return rot;
    }

    /*template<typename... Args, size_t... I>
    static std::tuple<Args...> deserializeTupleImpl(const uint8_t* p, std::index_sequence<I...>) {
        return std::make_tuple(deserializeOne<Args>(p)...);
    }*/
    template<typename... Args, size_t... I>
    static std::tuple<Args...> deserializeTupleImpl(const uint8_t* p, std::index_sequence<I...>) 
    {
        std::tuple<Args...> result;
        ((std::get<I>(result) = deserializeOne<Args>(p)), ...); // évalue dans l’ordre
        return result;
    }

    template<typename... Args>
    static std::tuple<Args...> deserializeTuple(const RawData& buf) {
        const uint8_t* p = buf.data();
        return deserializeTupleImpl<Args...>(p, std::index_sequence_for<Args...>{});
    }
};
