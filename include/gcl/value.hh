// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace gcl {

class Value;

using String = std::string;
using Array = std::vector<Value>;
using Dict = std::map<std::string, Value>;

enum class ValueType {
    Undefined,
    Null,
    Bool,
    Int,
    Float,
    String,
    Array,
    Dict,
};

union ValueData {
    constexpr ValueData() : i{0} {}
    ~ValueData() {}

    bool b;
    intptr_t i;
    float f;
    String string;
    Array array;
    Dict dict;
};

class IValueVisitor {
public:
    virtual void visit() = 0;
    virtual void visit(std::nullptr_t) = 0;
    virtual void visit(bool x) = 0;
    virtual void visit(intptr_t x) = 0;
    virtual void visit(float x) = 0;
    virtual void visit(String const& x) = 0;
    virtual void visit(Array const& x) = 0;
    virtual void visit(Dict const& x) = 0;
};

class Value {
public:
    ValueType type;
    ValueData data;

    constexpr Value() : type{ValueType::Undefined}, data{} {}
    constexpr Value(std::nullptr_t) : type{ValueType::Null}, data{} {}

    inline Value(bool x) : type{ValueType::Bool} {
        data.b = x;
    }

    inline Value(intptr_t x) : type{ValueType::Int} {
        data.i = x;
    }

    inline Value(float x) : type{ValueType::Float} {
        data.f = x;
    }

    inline Value(String&& x) : type{ValueType::String} {
        new (&data.string) String(std::move(x));
    }

    inline Value(Array&& x) : type{ValueType::Array} {
        new (&data.array) Array(std::move(x));
    }

    inline Value(Dict&& x) : type{ValueType::Dict} {
        new (&data.dict) Dict(std::move(x));
    }

    inline Value(Value const& that) : type{that.type} {
        that.copyDataTo(data);
    }

    inline Value(Value&& that) : type{that.type} {
        that.moveDataTo(data);
        that.type = ValueType::Undefined;
    }

    inline ~Value() {
        if (type != ValueType::Undefined)
            releaseData();
    }

    inline void clear() {
        if (type != ValueType::Undefined) {
            releaseData();
            type = ValueType::Undefined;
        }
    }

    inline Value& operator =(Value const& that) {
        if (type != ValueType::Undefined)
            releaseData();

        type = that.type;
        that.copyDataTo(data);

        return *this;
    }

    inline Value& operator =(Value&& that) {
        if (type != ValueType::Undefined)
            releaseData();

        type = that.type;
        that.moveDataTo(data);
        that.type = ValueType::Undefined;

        return *this;
    }

    inline bool isNull() const {
        return type == ValueType::Null;
    }

    inline bool getBool() {
        if (type != ValueType::Bool)
            throw std::runtime_error("Value::getBool() -> type mismatch");

        return data.b;
    }

    inline intptr_t getInt() {
        if (type != ValueType::Int)
            throw std::runtime_error("Value::getInt() -> type mismatch");

        return data.i;
    }

    inline float getFloat() {
        if (type != ValueType::Float)
            throw std::runtime_error("Value::getFloat() -> type mismatch");

        return data.f;
    }

    inline String& getString() {
        if (type != ValueType::String)
            throw std::runtime_error("Value::getString() -> type mismatch");

        return data.string;
    }

    inline Array& getArray() {
        if (type != ValueType::Array)
            throw std::runtime_error("Value::getArray() -> type mismatch");

        return data.array;
    }

    inline Dict& getDict() {
        if (type != ValueType::Dict)
            throw std::runtime_error("Value::getDict() -> type mismatch");

        return data.dict;
    }

    void accept(IValueVisitor& visitor) const {
        switch (type) {
            case ValueType::Bool:
                visitor.visit(data.b);
                break;

            case ValueType::Int:
                visitor.visit(data.i);
                break;

            case ValueType::Float:
                visitor.visit(data.f);
                break;

            case ValueType::String:
                visitor.visit(data.string);
                break;

            case ValueType::Array:
                visitor.visit(data.array);
                break;

            case ValueType::Dict:
                visitor.visit(data.dict);
                break;

            case ValueType::Null:
                visitor.visit(nullptr);
                break;

            case ValueType::Undefined:
                visitor.visit();
                break;
        }
    }

private:
    void copyDataTo(ValueData& dest) const {
        switch (type) {
            case ValueType::Bool:
                dest.b = data.b;
                break;

            case ValueType::Int:
                dest.i = data.i;
                break;

            case ValueType::Float:
                dest.f = data.f;
                break;

            case ValueType::String:
                new (&dest.string) String(data.string);
                break;

            case ValueType::Array:
                new (&dest.array) Array(data.array);
                break;

            case ValueType::Dict:
                new (&dest.dict) Dict(data.dict);
                break;

            default:
                dest.i = 0;
                break;
        }
    }

    void moveDataTo(ValueData& dest) {
        switch (type) {
            case ValueType::Bool:
                dest.b = data.b;
                break;

            case ValueType::Int:
                dest.i = data.i;
                break;

            case ValueType::Float:
                dest.f = data.f;
                break;

            case ValueType::String:
                new (&dest.string) String(std::move(data.string));
                break;

            case ValueType::Array:
                new (&dest.array) Array(std::move(data.array));
                break;

            case ValueType::Dict:
                new (&dest.dict) Dict(std::move(data.dict));
                break;

            default:
                dest.i = 0;
                break;
        }
    }

    void releaseData() {
        switch (type) {
            case ValueType::String:
                data.string.~String();
                break;

            case ValueType::Array:
                data.array.~Array();
                break;

            case ValueType::Dict:
                data.dict.~Dict();
                break;

            default:
                break;
        }
    }
};

} // namespace gcl
