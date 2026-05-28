#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <fstream>
#include <variant>
#include <iterator>
#include <cctype>

namespace nlohmann {

class json {
public:
    enum value_t {
        null,
        object,
        array,
        string,
        boolean,
        number_integer,
        number_float
    };

    json() : type_(null) {}
    json(const char* s) : type_(string), string_value_(s ? s : "") {}
    json(const std::string& s) : type_(string), string_value_(s) {}
    json(int v) : type_(number_integer), int_value_(v) {}
    json(int64_t v) : type_(number_integer), int_value_(v) {}
    json(double v) : type_(number_float), float_value_(v) {}
    json(bool v) : type_(boolean), bool_value_(v) {}

    static json make_array() { json j; j.type_ = array; return j; }
    static json make_object() { json j; j.type_ = object; return j; }

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = json;
        using difference_type = ptrdiff_t;
        using pointer = json*;
        using reference = json&;
        using array_iterator = typename std::vector<json>::iterator;
        using object_iterator = typename std::map<std::string, json>::iterator;

        iterator() : variant_(array_iterator{}) {}
        iterator(array_iterator it) : variant_(it) {}
        iterator(object_iterator it) : variant_(it) {}

        reference operator*() {
            if (std::holds_alternative<array_iterator>(variant_)) {
                return *std::get<array_iterator>(variant_);
            } else {
                return std::get<object_iterator>(variant_)->second;
            }
        }

        pointer operator->() { return &(**this); }

        iterator& operator++() {
            if (std::holds_alternative<array_iterator>(variant_)) {
                ++std::get<array_iterator>(variant_);
            } else {
                ++std::get<object_iterator>(variant_);
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            if (variant_.index() != other.variant_.index()) return false;
            if (std::holds_alternative<array_iterator>(variant_)) {
                return std::get<array_iterator>(variant_) == std::get<array_iterator>(other.variant_);
            } else {
                return std::get<object_iterator>(variant_) == std::get<object_iterator>(other.variant_);
            }
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        const std::string& key() const {
            return std::get<object_iterator>(variant_)->first;
        }

        reference value() const {
            return const_cast<iterator*>(this)->operator*();
        }

        std::variant<array_iterator, object_iterator> variant_;
    };

    class const_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = const json;
        using difference_type = ptrdiff_t;
        using pointer = const json*;
        using reference = const json&;
        using array_iterator = typename std::vector<json>::const_iterator;
        using object_iterator = typename std::map<std::string, json>::const_iterator;

        const_iterator() : variant_(array_iterator{}) {}
        const_iterator(array_iterator it) : variant_(it) {}
        const_iterator(object_iterator it) : variant_(it) {}
        const_iterator(const iterator& it) {
            if (std::holds_alternative<iterator::array_iterator>(it.variant_)) {
                variant_ = std::get<iterator::array_iterator>(it.variant_);
            } else {
                variant_ = std::get<iterator::object_iterator>(it.variant_);
            }
        }

        reference operator*() const {
            if (std::holds_alternative<array_iterator>(variant_)) {
                return *std::get<array_iterator>(variant_);
            } else {
                return std::get<object_iterator>(variant_)->second;
            }
        }

        pointer operator->() const { return &(**this); }

        const_iterator& operator++() {
            if (std::holds_alternative<array_iterator>(variant_)) {
                ++std::get<array_iterator>(variant_);
            } else {
                ++std::get<object_iterator>(variant_);
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const {
            if (variant_.index() != other.variant_.index()) return false;
            if (std::holds_alternative<array_iterator>(variant_)) {
                return std::get<array_iterator>(variant_) == std::get<array_iterator>(other.variant_);
            } else {
                return std::get<object_iterator>(variant_) == std::get<object_iterator>(other.variant_);
            }
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

        const std::string& key() const {
            return std::get<object_iterator>(variant_)->first;
        }

        reference value() const {
            return operator*();
        }

    private:
        std::variant<array_iterator, object_iterator> variant_;
    };

    iterator begin() {
        if (type_ == array) return iterator(array_values_.begin());
        if (type_ == object) return iterator(object_values_.begin());
        return iterator(array_values_.end());
    }

    iterator end() {
        if (type_ == array) return iterator(array_values_.end());
        if (type_ == object) return iterator(object_values_.end());
        return iterator(array_values_.end());
    }

    const_iterator begin() const {
        if (type_ == array) return const_iterator(array_values_.begin());
        if (type_ == object) return const_iterator(object_values_.begin());
        return const_iterator(array_values_.end());
    }

    const_iterator end() const {
        if (type_ == array) return const_iterator(array_values_.end());
        if (type_ == object) return const_iterator(object_values_.end());
        return const_iterator(array_values_.end());
    }

    bool is_array() const { return type_ == array; }
    bool is_object() const { return type_ == object; }
    bool is_string() const { return type_ == string; }
    bool is_number() const { return type_ == number_integer || type_ == number_float; }
    bool is_boolean() const { return type_ == boolean; }
    bool is_null() const { return type_ == null; }

    std::string value(const char* key, const char* defaultValue) const {
        if (type_ != object) return defaultValue;
        auto it = object_values_.find(key);
        if (it == object_values_.end()) return defaultValue;
        if (!it->second.is_string()) return defaultValue;
        return it->second.string_value_;
    }

    std::string value(const std::string& key, const std::string& defaultValue) const {
        return value(key.c_str(), defaultValue.c_str());
    }

    json& operator[](const char* key) {
        type_ = object;
        return object_values_[key];
    }

    json& operator[](const std::string& key) {
        return operator[](key.c_str());
    }

    json& operator[](size_t index) {
        type_ = array;
        if (index >= array_values_.size()) {
            array_values_.resize(index + 1);
        }
        return array_values_[index];
    }

    void push_back(const json& value) {
        type_ = array;
        array_values_.push_back(value);
    }

    size_t size() const {
        if (type_ == array) return array_values_.size();
        if (type_ == object) return object_values_.size();
        return 0;
    }

    bool empty() const { return size() == 0; }

    std::string dump(int indent = -1) const {
        std::ostringstream oss;
        dump(oss, indent, 0);
        return oss.str();
    }

    template<typename T>
    T get() const { return T(); }

    friend std::istream& operator>>(std::istream& is, json& j) {
        std::stringstream ss;
        ss << is.rdbuf();
        std::string s = ss.str();
        
        size_t pos = 0;
        while (pos < s.size() && std::isspace(s[pos])) pos++;
        
        if (pos >= s.size()) {
            j = json();
            return is;
        }
        
        if (s[pos] == '[') {
            j.type_ = array;
        } else if (s[pos] == '{') {
            j.type_ = object;
        } else if (s[pos] == '"') {
            j.type_ = string;
            size_t end = s.find('"', pos + 1);
            if (end != std::string::npos) {
                j.string_value_ = s.substr(pos + 1, end - pos - 1);
            }
        }
        
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        os << j.dump();
        return os;
    }

    value_t type_ = null;
    std::string string_value_;
    int64_t int_value_ = 0;
    double float_value_ = 0.0;
    bool bool_value_ = false;
    std::vector<json> array_values_;
    std::map<std::string, json> object_values_;

private:
    void dump(std::ostringstream& oss, int indent, int level) const {
        std::string indentStr;
        if (indent > 0) {
            indentStr = std::string(level * indent, ' ');
        }

        switch (type_) {
            case null:
                oss << "null";
                break;
            case boolean:
                oss << (bool_value_ ? "true" : "false");
                break;
            case number_integer:
                oss << int_value_;
                break;
            case number_float:
                oss << float_value_;
                break;
            case string:
                oss << "\"" << string_value_ << "\"";
                break;
            case array:
                oss << "[";
                if (indent > 0) oss << "\n";
                for (size_t i = 0; i < array_values_.size(); ++i) {
                    if (i > 0) oss << ",";
                    if (indent > 0) oss << "\n" << std::string((level + 1) * indent, ' ');
                    array_values_[i].dump(oss, indent, level + 1);
                }
                if (indent > 0 && !array_values_.empty()) oss << "\n" << indentStr;
                oss << "]";
                break;
            case object:
                oss << "{";
                if (indent > 0) oss << "\n";
                bool first = true;
                for (const auto& pair : object_values_) {
                    if (!first) oss << ",";
                    if (indent > 0) oss << "\n" << std::string((level + 1) * indent, ' ');
                    oss << "\"" << pair.first << "\":";
                    if (indent > 0) oss << " ";
                    pair.second.dump(oss, indent, level + 1);
                    first = false;
                }
                if (indent > 0 && !object_values_.empty()) oss << "\n" << indentStr;
                oss << "}";
                break;
        }
    }
};

template<>
inline std::string json::get<std::string>() const {
    if (type_ == string) return string_value_;
    std::ostringstream oss;
    dump(oss, -1, 0);
    return oss.str();
}

template<>
inline int json::get<int>() const {
    if (type_ == number_integer) return (int)int_value_;
    if (type_ == number_float) return (int)float_value_;
    return 0;
}

template<>
inline int64_t json::get<int64_t>() const {
    if (type_ == number_integer) return int_value_;
    if (type_ == number_float) return (int64_t)float_value_;
    return 0;
}

template<>
inline bool json::get<bool>() const {
    if (type_ == boolean) return bool_value_;
    return false;
}

}

#endif
