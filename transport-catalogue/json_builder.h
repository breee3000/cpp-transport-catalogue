#pragma once

#include "json.h"

namespace json {

class KeyItemContext;
class ValueItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder();
    KeyItemContext Key(std::string);
    Builder& Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
};

class ItemContext {
public:
    ItemContext(Builder& builder)
        : builder_(builder) {
    };

    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value val);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
private:
    Builder& builder_;
};


class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder)
        : ItemContext(builder) {
    };

    KeyItemContext Key(std::string) = delete;
    ValueItemContext Value(Node::Value val);
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class ValueItemContext : public ItemContext {
public:
    ValueItemContext(Builder& builder)
        : ItemContext(builder) {
    };

    Builder& Value(Node::Value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder)
        : ItemContext(builder) {
    };

    Builder& Value(Node::Value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder)
        : ItemContext(builder) {
    };

    KeyItemContext Key(std::string) = delete;
    ArrayItemContext Value(Node::Value val);
    Builder& EndDict() = delete;
};

} //namespace json
