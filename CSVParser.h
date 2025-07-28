/* ======= github.com/vlaxcs ======= */

#pragma once

#include <utility>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <exception>
#include <format>
#include <set>

/* ======= Allowed containers ======= */
template<typename T>
struct is_allowed_container : std::false_type {};

template<typename T>
struct is_allowed_container<std::vector<T>> : std::true_type {};

template<typename T>
struct is_allowed_container<std::set<T>> : std::true_type {};

template<typename TObject, template<typename> class Container>
concept AllowedContainer = is_allowed_container<Container<TObject>>::value;

template<typename T>
concept HasIdMember = requires(T t) {
    { t.id } -> std::convertible_to<int>;
};


/* ======= All CSV Exceptions ======= */
template<typename TObject, typename... Types>
    requires(sizeof...(Types) > 0)
class CSVParser;

class CSVException : public std::exception {
private:
    template<typename TObject, typename... Types>
        requires(sizeof...(Types) > 0)
    friend class CSVParser;

protected:
    explicit CSVException(std::string msg) : message(std::move(msg)) {
    }

    std::string message;
    static std::string inline error_mark = "[CSV Parser ERROR]";

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
};

// Illegal: Cannot open a file with provided filename.
class FileOpenException final : public CSVException {
private:
    template<typename TObject, typename... Types>
        requires(sizeof...(Types) > 0)
    friend class CSVParser;

    explicit FileOpenException(const std::string &filename)
        : CSVException(std::format("{} Failed to open file: {}", error_mark, filename)) {
    }
};

// Illegal: Using custom delimiter, cannot find a header of length equal to custom header's length.
class WrongHeaderByDelimiter final : public CSVException {
    template<typename TObject, typename... Types>
        requires(sizeof...(Types) > 0)
    friend class CSVParser;

    WrongHeaderByDelimiter(const std::string &filename, size_t detected_size, size_t expected_size, size_t row, char delimiter)
        : CSVException(std::format(
            "{} Failed to match header of size [{}] using delimiter '{}' on row [{}] in file '{}'.\n User's header has size {}.",
            error_mark, detected_size, delimiter, row, filename, expected_size)) {
    }
};

// Illegal: At least one default delimiter should match the custom header.
class WrongHeaderByAllDelimiters final : public CSVException {
    template<typename TObject, typename... Types>
        requires(sizeof...(Types) > 0)
    friend class CSVParser;

    WrongHeaderByAllDelimiters(
        const std::string &filename,
        const std::unordered_map<char, std::pair<int, std::vector<std::string> > > &detected_values,
        const size_t expected_size,
        const size_t row_number
    ) : CSVException(buildMessage(filename, detected_values, expected_size, row_number)) {
    }

    static std::string buildMessage(
        const std::string &filename,
        const std::unordered_map<char, std::pair<int, std::vector<std::string> > > &detected_values,
        size_t expected_size,
        size_t row_number
    ) {
        std::ostringstream oss;

        oss << std::format(
            "{} Found a header mismatch on row [{}] in file '{}'. Expected size: [{}].\n",
            error_mark, row_number, filename, expected_size
        );

        for (const auto &[delimiter, pair]: detected_values) {
            oss << std::format("{} Found header of size [{}] delimited by '{}'\n[CSV Parser ERROR]\t",
                               error_mark, pair.first, delimiter
            );

            for (const auto &tag: pair.second) {
                oss << tag << ' ';
            }

            oss << "\n";
        }

        return oss.str();
    }
};

// Illegal: The length of the header is different from the maximum arity of object's constructors.
class WrongHeaderLength final : public CSVException {
    template<typename TObject, typename... Types>
        requires(sizeof...(Types) > 0)
    friend class CSVParser;

    WrongHeaderLength(
        const std::size_t expected_size,
        const std::vector<std::string> &header
    ) : CSVException(buildMessage(expected_size, header)) {
    }

    static std::string buildMessage(
        const std::size_t expected_size,
        const std::vector<std::string> &header
    ) {
        std::ostringstream oss;
        oss << std::format(
            "{} In header: ",
            error_mark);

        for (const auto &head: header) {
            oss << std::format("'{}'", head) << " ";
        }

        oss << std::format(
            "\n{} The length of the header [{}] is different from the maximum arity of object's constructors [{}].\n",
            error_mark,
            header.size(),
            expected_size
        );

        return oss.str();
    }
};


/* ======= Helper functions for Unique Type parsing ======= */

// Uses the unique type if the size of 'Types...' (from CSVParser object's template) is 1.
template<typename... Types>
struct front_type;

template<typename First, typename... Rest>
struct front_type<First, Rest...> {
    using type = First;
};

// Checks if object of type TObject has a constructor of arity N and all arguments of type ArgType.
template<typename TObject, typename ArgType, std::size_t N>
struct isConstructibleWithNArgsUniqueType {
private:
    template<std::size_t... Index>
    static auto test(std::index_sequence<Index...>)
        -> std::is_constructible<TObject, decltype((void) Index, ArgType{})...>;

public:
    static constexpr bool value = decltype(test(std::make_index_sequence<N>{}))::value;
};

// Checks if object of type TObject has a constructor of arity from Max to 1 and all arguments of type ArgType. Given the order, will hold the maximum arity found.
template<typename TObject, typename ArgType, std::size_t Max>
constexpr std::size_t getMaxConstructibleArityUniqueType() {
    if constexpr (Max == 0) {
        return 0;
    } else if constexpr (isConstructibleWithNArgsUniqueType<TObject, ArgType, Max>::value) {
        return Max;
    } else {
        return getMaxConstructibleArityUniqueType<TObject, ArgType, Max - 1>();
    }
}


/* ======= Helper functions for Multiple Type parsing ======= */

// Checks if object of type TObject has a constructor of arity N and all arguments of types from ArgTypes, in order.
template<typename TObject, std::size_t N, typename... ArgTypes>
struct isConstructibleWithNArgsMultipleTypes {
private:
    template<std::size_t... Index>
    static auto test(std::index_sequence<Index...>)
        -> std::is_constructible<TObject, std::tuple_element_t<Index, std::tuple<ArgTypes...> >...>;

public:
    static constexpr bool value = decltype(test(std::make_index_sequence<N>{}))::value;
};

// Checks if object of type TObject has a constructor of arity from Max to 1 and all arguments of types from ArgTypes, in order. Given the order, will hold the maximum arity found.
template<typename TObject, std::size_t Max, typename... ArgTypes>
constexpr std::size_t getMaxConstructibleArityMultipleTypes() {
    if constexpr (Max == 0) {
        return 0;
    } else if constexpr (isConstructibleWithNArgsMultipleTypes<TObject, Max, ArgTypes...>::value) {
        return Max;
    } else {
        return getMaxConstructibleArityMultipleTypes<TObject, Max - 1, ArgTypes...>();
    }
}


/* ======= CSVParser class definition ======= */

template<typename TObject, typename... Types>
    requires(sizeof...(Types) > 0)
class CSVParser {
private:
    using front_t = typename front_type<Types...>::type;

    static constexpr std::size_t max_args_unique_type = getMaxConstructibleArityUniqueType<TObject, front_t, 40>();
    static constexpr std::size_t max_args_multiple_types = getMaxConstructibleArityMultipleTypes<TObject, sizeof...(Types), Types...>();

    static inline std::vector<char> default_delimiters = {',', '\t', ';', '|', ':', ' ', '~'};
    static inline std::vector<char> default_quotes = {'"', '\''};

    std::vector<std::string> header;
    bool custom_header = false, has_id = false;
    static inline int objectIdCounter = 0;
    char delimiter, quote;
    int header_row;

    [[nodiscard]] bool checkMaxArgs(const std::size_t value) const {
        switch (sizeof...(Types)) {
            case 1:
                return (max_args_unique_type == value || std::is_same_v<TObject, std::vector<front_t>>);
            default:
                return max_args_multiple_types == value;
        }
    }

    [[nodiscard]] bool checkMaxArgs(const std::vector<std::string>& temp) const {
        switch (sizeof...(Types)) {
            case 1:
                if (max_args_unique_type != temp.size() && !std::is_same_v<TObject, std::vector<front_t> >) {
                    throw WrongHeaderLength(max_args_unique_type, temp);
                }
            break;
            default:
                if (max_args_multiple_types != temp.size()) {
                    throw WrongHeaderLength(max_args_multiple_types, temp);
                }
        }
        return true;
    }

    void setHeader(const std::vector<std::string>& temp) {
        if (checkMaxArgs(temp)) {
            header = temp;
        }
    }

    static bool getBoolMeaning(const std::string &trimmed_boolean) {
        std::string lower_temp = trimmed_boolean;
        std::ranges::transform(lower_temp, lower_temp.begin(), ::tolower);

        return (lower_temp[0] == 'y' || lower_temp[0] == 't' || lower_temp[0] == '1');
    }

    // Determines if the original header can be replaced with user's preferences.
    // A function which checks and eventually predicts custom headers.
    [[nodiscard]] std::pair<bool, char> trust_header(const std::string &filename);

    // Parses a single CSV formatted row.
   TObject parseObjectFromRow(const std::string &row);

    void showStats(const std::string& filename) {
        std::cout << std::format("[CSV Reader] Fetching data from {}", filename) << std::endl;
        if (custom_header) {
            std::cout << std::format("[CSV Reader] Set header to user's preferences.") << std::endl;
        } else {
            std::cout << std::format("[CSV Reader] Set header from CSV (row: {} | delimiter: '{}')", header_row,
                                     delimiter) << std::endl;
        }
        std::cout << std::format("[CSV Reader] Will retrieve data from the first {} columns, separated by '{}'",
                                 header.size(), delimiter) << std::endl;
    }

public:
    // Create a CSV Parser using a predefined header.
    explicit CSVParser(const std::vector<std::string> &header_)
        : header({}),
          has_id(HasIdMember<TObject>),
          delimiter('\0'),
          quote('\0'),
          header_row(1){
        try {
            setHeader(header_);
        } catch (WrongHeaderLength&) {
            throw;
        }
    }

    // Create a CSV Parser using default CSV file's header.
    explicit CSVParser()
        : header({}), has_id(HasIdMember<TObject>), delimiter('\0'), quote('\0'),  header_row(1){
    }

    // Set the CSV file's delimiter symbol.
    void setDelimiter(const char delimiter_symbol) {
        delimiter = delimiter_symbol;
    }

    // Set the CSV file's quotation symbol.
    void setQuote(const char quotation_symbol) {
        quote = quotation_symbol;
    }

    // Set the CSV file's header row. Index should start from 1 (Not critical validation).
    void setHeaderRow(const int row) {
        if (row < 1) {
            std::clog << std::format("[CSV Parser Error] Failed to set header row to [{}]. Reason: Index should start from 1.", row) << std::endl;
            return;
        }
        header_row = row;
    }

    template<template<typename> class Container>
        requires AllowedContainer<TObject, Container>
    Container<TObject> parseObjectsFromFile(const std::string &filename);

    ~CSVParser() = default;
};


template<typename TObject, typename... Types>
    requires(sizeof...(Types) > 0)
std::pair<bool, char> CSVParser<TObject, Types...>::trust_header(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw FileOpenException(filename);
    }

    int current_row(1);
    std::string row;

    do {
        std::getline(file, row);
        current_row++;
    } while (current_row == header_row);

    // If the delimiter is defined...
    if (delimiter != '\0') {
        std::stringstream ss(row);
        std::string cell;
        std::vector<std::string> try_header;

        while (std::getline(ss, cell, delimiter)) {
            // try_header.push_back(parseCell(ss, cell, delimiter));
            try_header.push_back(cell);
        }

        // If the custom header is not defined, set the default header from CSV file.
        if (header.empty()) {
            try {
                setHeader(try_header);
                return {false, delimiter};
            } catch (WrongHeaderLength&) {
                throw;
            }
        }

        // If the custom header is defined, the new custom header must be of same size.
        if (try_header.size() == header.size()) {
            return {true, delimiter};
        }

        throw WrongHeaderByDelimiter(filename, try_header.size(), header.size(), header_row, delimiter);
    }

    // Else, if the delimiter is not defined...
    std::unordered_map<char, std::pair<int, std::vector<std::string> > > detected_values;
    for (const char current_delimiter: default_delimiters) {
        std::stringstream ss(row);
        std::string cell;
        std::vector<std::string> try_header;

        while (std::getline(ss, cell, current_delimiter)) {
            // try_header.push_back(parseCell(ss, cell, delimiter));
            try_header.push_back(cell);
        }

        if (try_header.size() == header.size() && !header.empty()) {
            setHeader(try_header);
            return {true, current_delimiter};
        }

        detected_values[current_delimiter] = {try_header.size(), try_header};
    }

    // If no header is defined, all default delimiters will be checked,
    // and the parser will assume the number of columns in the CSV
    // matches the number of fields (arity) of the object.
    if (header.empty()) {
        // Read next row
        int length(0);
        char good_delimiter('\0');
        std::getline(file, row);
        std::vector<std::pair<int, char> > detected_values_next_row;

        for (const char current_delimiter: default_delimiters) {
            std::stringstream ss(row);
            std::string cell;
            int value_counter(0);

            while (std::getline(ss, cell, current_delimiter)) {
                value_counter++;
            }

            if (detected_values[current_delimiter].first == value_counter
                && value_counter > 0
                && checkMaxArgs(value_counter)) {
                good_delimiter = current_delimiter;
                length = value_counter;
                break;
            }
        }

        if (length) {
            try {
                setHeader(detected_values[good_delimiter].second);
                return {false, good_delimiter};
            } catch (WrongHeaderLength&) {
                throw;
            }
        }
    }

    throw WrongHeaderByAllDelimiters(filename, detected_values, header.size(), header_row);
}


template<typename TObject, typename... Types>
    requires(sizeof...(Types) > 0)
template<template<typename> class Container>
    requires AllowedContainer<TObject, Container>
Container<TObject> CSVParser<TObject, Types...>::parseObjectsFromFile(const std::string &filename) {
    try {
        const std::pair<int, char> parseType(this->trust_header(filename));

        custom_header = parseType.first;
        delimiter = parseType.second;

        std::ifstream file(filename);
        if (!file.is_open()) {
            throw FileOpenException(filename);
        }

        showStats(filename);

        // Consume all rows before header.
        std::string row;
        int row_counter(1);
        do {
            std::getline(file, row);
            row_counter++;
        } while (row_counter == header_row);

        // Retrieves data from a row and add the object in the container.
        Container<TObject> result;

        if constexpr (std::is_same_v<Container<TObject>, std::vector<TObject>>) {
            while (std::getline(file, row)) {
                if (row.empty()) {
                    continue;
                }
                TObject newObject = this->parseObjectFromRow(row);
                result.push_back(std::move(newObject));
            }
        } else if constexpr (std::is_same_v<Container<TObject>, std::set<TObject>>) {
            while (std::getline(file, row)) {
                if (row.empty()) {
                    continue;
                }
                TObject newObject = this->parseObjectFromRow(row);
                result.emplace(std::move(newObject));
            }
        } else if constexpr (std::is_same_v<Container<TObject>, std::unordered_map<int, TObject>>) {
            while (std::getline(file, row)) {
                if (row.empty()) {
                    continue;
                }
                TObject newObject = this->parseObjectFromRow(row);
                result[has_id ? newObject.id : ++objectIdCounter] = newObject;
            }
        }

        return result;

    } catch (const WrongHeaderLength&) {
        throw;
    } catch (const CSVException &exception) {
        std::clog << exception.what() << std::endl;
        return {};
    } catch (...) {
        std::clog << "[CSV Parser ERROR] Unexpected exception has occurred." << std::endl;
        return {};
    }
}


/* ======= Parsing Unique Type object ======= */

template<typename TObject, typename UniqueType, std::size_t... Is>
TObject constructObjectFromVectorUniqueType(const std::vector<UniqueType> &vec, std::index_sequence<Is...>) {
    return TObject(vec[Is]...);
}

template<typename TObject, typename UniqueType, std::size_t N>
TObject constructObjectUniqueTypeArgs(const std::vector<UniqueType> &vec) {
    if (vec.size() < N)
        throw std::invalid_argument("Not enough arguments in vector");
    return constructObjectFromVectorUniqueType<TObject>(vec, std::make_index_sequence<N>{});
}


/* ======= Parsing Multiple Type object ======= */
struct NodeBase {
    NodeBase *prev = nullptr;
    NodeBase *next = nullptr;
    std::string type;

    virtual ~NodeBase() = default;
};

template<typename T>
struct NodeMT final : NodeBase {
    std::tuple<T> single_tuple;
};

template<typename Head>
void parseMTRow(std::stringstream &ss, const char delimiter, NodeBase *&current) {
    std::string cell;
    std::getline(ss, cell, delimiter);

    auto *node = new NodeMT<Head>();

    try {
        std::istringstream iss(cell);
        Head value;
        iss >> value;
        if (iss.fail()) {
            throw std::runtime_error("Parse error");
        }
        node->single_tuple = std::make_tuple(value);
    } catch (...) {
        node->single_tuple = std::make_tuple(Head{});
    }

    node->type = typeid(Head).name();

    if (current) {
        current->next = node;
        node->prev = current;
    }

    current = node;
}

template<typename Head, typename... Tail>
    requires(sizeof...(Tail) > 0)
void parseMTRow(std::stringstream &ss, const char delimiter, NodeBase *&current) {
    std::string cell;
    std::getline(ss, cell, delimiter);

    auto *node = new NodeMT<Head>();

    try {
        std::istringstream iss(cell);
        Head value;
        iss >> value;
        if (iss.fail()) {
            throw std::runtime_error("Parse error");
        }
        node->single_tuple = std::make_tuple(value);
    } catch (...) {
        node->single_tuple = std::make_tuple(Head{});
    }

    node->type = typeid(Head).name();

    if (current) {
        current->next = node;
        node->prev = current;
    }

    current = node;

    if (sizeof...(Tail) > 0) {
        parseMTRow<Tail...>(ss, delimiter, current);
    }
}

template<typename T>
T extractAndAdvance(NodeBase *&node) {
    auto *typed = dynamic_cast<NodeMT<T> *>(node);
    if (!typed) {
        throw std::runtime_error("Type mismatch during object construction.");
    }
    T val = std::get<0>(typed->single_tuple);
    node = node->prev;
    return val;
}

template<typename... Types>
std::tuple<Types...> buildTupleFromNodes(NodeBase *&node) {
    return std::make_tuple(extractAndAdvance<Types>(node)...);
}


// Parse each row
template<typename TObject, typename... Types>
    requires(sizeof...(Types) > 0)
TObject CSVParser<TObject, Types...>::parseObjectFromRow(const std::string &row) {
    // TODO VERIFY IF BOOL ON CELLS, NOT ON ROWS - Do the proper cell reading
    if constexpr (sizeof...(Types) == 1) {
        std::vector<front_t> temp_values;
        std::stringstream ss(row);
        std::string cell;
        int current_cell_count(0);

        while (std::getline(ss, cell, delimiter) && current_cell_count < header.size()) {
            try {
                std::istringstream iss(cell);
                front_t value;
                iss >> value;

                if (iss.fail()) {
                    throw std::runtime_error("Parse error");
                }

                temp_values.push_back(value);
            } catch (std::exception &) {
                temp_values.push_back(front_t(0));
            }

            current_cell_count++;
        }

        if constexpr (std::is_same_v<TObject, std::vector<front_t>>) {
            return temp_values;
        } else {
            return constructObjectUniqueTypeArgs<TObject, front_t, max_args_unique_type>(temp_values);
        }
    } else {
        std::stringstream ss(row);
        NodeBase *current = nullptr;
        parseMTRow<Types...>(ss, delimiter, current);

        NodeBase *node = current;

        auto tuple = buildTupleFromNodes<Types...>(node);

        while (node) {
            NodeBase *prev = node->prev;
            delete prev;
            node = prev;
        }

        return std::make_from_tuple<TObject>(tuple);
    }
}