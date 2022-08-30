#include "json_builder.h"

namespace json {

using namespace std::literals;

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}

KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Incorrect Key()");
    }
    nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key]);
    return *this;
}

Builder& Builder::Value(Node::Value val) {
    if (nodes_stack_.empty() ||
            (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Incorrect Value()");
    }
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Node{});
        const_cast<Array&>(nodes_stack_.back()->AsArray()).back().GetValue() = val;
    } else {
        nodes_stack_.back()->GetValue() = val;
        nodes_stack_.pop_back();
    }
    return *this;
}

DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty() ||
            (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Incorrect StartDict()");
    }
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Dict{});
        nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
    } else {
        *nodes_stack_.back() = Dict{};
    }
    return *this;
}

ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty() ||
            (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Incorrect StartArray()");
    }
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array{});
        nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
    } else {
        *nodes_stack_.back() = Array{};
    }
    return *this;
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Incorrect EndDict()");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Incorrect EndArray()");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Incorrect Build()");
    }
    return root_;
}

KeyItemContext ItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& ItemContext::Value(Node::Value val) {
    return builder_.Value(std::move(val));
}

DictItemContext ItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ItemContext::EndDict() {
    return builder_.EndDict();
}

Builder& ItemContext::EndArray() {
    return builder_.EndArray();
}

ValueItemContext KeyItemContext::Value(Node::Value val) {
    return ItemContext::Value(std::move(val));
}

ArrayItemContext ArrayItemContext::Value(Node::Value val) {
    return ItemContext::Value(std::move(val));
}

} //namespace json
