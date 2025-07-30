# CSV Object Parser C++ Library

***The first ever*** lightweight and header-only C++ library that parses CSV files and maps each row to a strongly-typed C++ object.\
It provides a simple and efficient way to deserialize structured CSV data into containers of custom objects.

**Be careful! CSV Object Parser library is limited to the functionalities described at **Syntax**, but should work properly if you follow all steps.**

> Launch date: 30.07.2025 \
> Last update: 30.07.2025

- ## Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Syntax](#syntax)
- [Benchmarks](#benchmarks)
- [Step by step example](#how-to-use-the-library-step-by-step-example)

- ## Requirements

### **Compatible with C++20 and higher**.

Used libraries: 
**[utility](https://en.cppreference.com/w/cpp/header/utility.html)**, 
**[vector](hhttps://en.cppreference.com/w/cpp/header/vector.html)**, 
**[string](https://en.cppreference.com/w/cpp/header/string.html)**, 
**[memory](https://en.cppreference.com/w/cpp/header/memory.html)**, 
**[unordered_map](https://en.cppreference.com/w/cpp/header/unordered_map.html)**, 
**[algorithm](https://en.cppreference.com/w/cpp/header/algorithm.html)**, 
**[fstream](https://en.cppreference.com/w/cpp/header/fstream.html)**, 
**[sstream](https://en.cppreference.com/w/cpp/header/sstream.html)**, 
**[exception](https://en.cppreference.com/w/cpp/header/exception.html)**, 
**[format](https://en.cppreference.com/w/cpp/header/format.html)**,
**[set](https://en.cppreference.com/w/cpp/header/set.html)**.

- ## Installation

1. Open your C++ project with CLion.
2. Create a new directory named `include`.
3. In CMakeLists.txt, add `target_include_directories(project_name PRIVATE include)` and replace `project_name` with the argument specified in `project(project_name)`.
4. In a new directory, clone the repository: `git clone https://github.com/vlaxcs/CSVObjectParser`
5. From this directory, move `CSVParser.h` into your `include` folder.
6. In the C++ source you want to process your objects, include the library using `#include <CSVParser.h>`.


- ## Syntax

### I. Template instantiation

There are 2 ways CSV Object Parser can approach reaching data from the CSV file.

1. Assumes that all arguments and all CSV cells (except header) can be converted to a **UniqueType**.
   - The longest constructor with arguments of same type will be reached. (???)
   - Signature:\
   `CSVParser<Object, UniqueType> object_parser;`
  
   - Example:\
   `CSVParser<Rating, float> object_parser;`


2. Needs to know all constructors' arguments' type.
   - **TypeK** belongs to **K-th** argument of Object's constructor, only if Object's constructor has **N** (1, 2, ... K ..., N - 1, N) arguments of types **TypeK**.
   - Signature:\
     `CSVParser<Object, Type1, Type2, Type3 ... TypeN> object_parser;`

   - Example:\
     `CSVParser<Object, int, float, string, AnotherObject> object_parser;`
        - If you need another object as argument (e.g. *AnotherObject*), parse it separately (Read **How to use**).

- Illegal: `CSVParser<Object>` with no type given. 
- Be aware of this example: `CSVParser<int, float>` will build **int objects**, assuming that all your CSV cells can be read as **float**.


### II. Parsing object instantiation

1. No header preferences. Will use CSV file's header and will establish the delimiter by the maximum frequency on first 2 rows of your document.
   - Constructor signature (default): `CSVParser()`
   - `CSVParser<...> object_parser;`

2. Using a custom header. Column names are not very important, but the length of header list should match the header length from CSV file.
   - Constructor signature: `CSVParser(const std::vector<std::string> &header_)`
   - Example: `CSVParser<...> object_parser({"First column", "Second column"});`
   - For these column names, CSV's file header must look like `text1,text2`, delimiter is not important in this example.

### III. Header properties
- Delimiter set:
    - No header preference -> Sets the header separated by the delimiter.
    - Header preference    -> Sets the header separated by the delimiter only if the lengths are equal ***or Throws on size mismatch***.

- Delimiter not set:
    - No header preference  -> Tries to find headers using default CSV delimiters. Sets the header of maximum matched length with next row's length (first row of values).
    - Header preference     -> Sets the header separated by the one of the default delimiters only if the lengths of file's header and preferred header are equal ***or Throws on size mismatch***.


### IV. Parsing object customization

1. Set the delimiter symbol.
   - `all_objects.setDelimiter(const char symbol)`

2. Set the quotation symbol.
   - `all_objects.setQuote(const char symbol)`

3. Set the header row index (Default indexed from 1. The custom header row index must start from 1).
   - `all_objects.setHeaderRow(const int)`
   

### V. Parsing from a file

#### 1. Containers of `shared_ptr<Object>`

- Result as **`std::vector<std::shared_ptr<Object>>`**
    - **Usage Syntax: `auto all_objects = object_parser.parsePointerObjectsFromFile<std::vector>(filename);`**


- Result as **`std::set<std::shared_ptr<Object>>`**
  - **Usage Syntax: `auto all_objects = object_parser.parsePointerObjectsFromFile<std::set>(filename);`**


- Result as **`std::unordered_map<KeyType, std::shared_ptr<Object>>`**
    - **Usage Syntax: `auto all_objects = object_parser.parsePointerObjectsFromFile<std::unordered_map, KeyType>(filename);`**


#### 2. Containers of `Object` (by value)

- Result as **`std::vector<Object>`**
    - **Usage Syntax: `auto all_objects = object_parser.parseObjectsFromFile<std::vector>(filename);`**


- Result as **`std::unordered_map<int, Object>`**
    - **Requirements:**
        - Public default constructor: `Object() = default;`
        - Private member: `YourType id;`
        - Public getter for **ID**: `YourType Object::getId() const { return Object.id };`
        - **ID** must be available from:
            - An **ID** column in CSV
            - Or an ID initialized from a **static ID counter** within the class constructor
    - **Usage Syntax: `auto all_objects = object_parser.parseObjectsFromFile<std::unordered_map, KeyType>(filename);`**


### VI. Container inspecting

- Requirements: a properly overload of **operator<<** for each containerized object.

- Containers of `shared_ptr<Object>`: `object_parser.inspect_pointers(container);`
- Containers of `Object` (by value): `object_parser.inspect(container);`


- ## Benchmarks

| **Container Type**   | **Ownership**         | **Objects Created** | **Total Time (s)** | **Time per Object (ms)** |
| -------------------- |-----------------------| ------------------- | ------------------ |-------------|
| `std::set`           | `std::shared_ptr<Ob>` | 10,000,000          | 46.768             | 0.0046768   |
| `std::vector`        | Ob(int,string,float)  | 10,000,000          | 34.409             | 0.0034409   |
| `std::vector`        | `std::shared_ptr<Ob>` | 10,000,000          | 36.423             | 0.0036423   |
| `std::unordered_map` | Ob(int,string,float)  | 10,000,000          | 38.230             | 0.0038230   |
| `std::unordered_map` | `std::shared_ptr<Ob>` | 10,000,000          | 38.704             | 0.0038704   |

*Compared to **OracleSQL**, adding 10.000.000 entities into a table with columns described as in the **Ob(int,string,float)** would take around **30s - 120s***.

- ## How to use the library (Step by step example)

**1. Create the objects. Ensure that you have a public constructor with all attributes you want to instance with CSV file's content.**

```
// Defining a subobject
class Room {
    // Mandatory for unordered_map
    int id;
    
    int house_id;
    std::string room_name;
    int capacity;
    float surface;

public:
    Room(int id_, const int house_id_, std::string  room_name_, const int capacity_, const float surface_)
        : id(id_), house_id(house_id_), room_name(std::move(room_name_)), capacity(capacity_), surface(surface_) {}

    // Mandatory for .inspect(...) and .inspect_pointers(...)
    friend std::ostream & operator<<(std::ostream &os, const Room &obj) {
        return os
               << "id: " << obj.id
               << "\thouse_id: " << obj.house_id
               << "\troom_name: " << obj.room_name
               << "\tcapacity: " << obj.capacity
               << "\tsurface: " << obj.surface;
    }

    // Mandatory for unordered_map
    int getId() const {
        return id;
    }

    // Mandatory for assigning to a house
    int getHouseId() {
        return house_id;
    }


    ~Room() = default;
};


// Defining the main object
class House {
private:
    // Mandatory for unordered_map
    int id;
    std::string address;
    int surface;
    
    // Container of subobjects
    std::vector<std::shared_ptr<Room>> rooms;

public:
    // Mandatory for set<Object>, vector<Object>, unordered_map<KeyType, Object>
    House() = default;

    House (const int id_, std::string address_, const int surface_)
        : id(id_), address(std::move(address_)), surface(surface_) {}

    // Mandatory for .inspect(...) and .inspect_pointers(...)
    friend std::ostream & operator<<(std::ostream &os, const House &obj) {
        os << "House:" << std::endl;

        os << "id: " << obj.id
           << "\taddress: " << obj.address
           << "\tsurface: " << obj.surface << std::endl;


        os << "Rooms: " << std::endl;
        for (const auto& room : obj.rooms) {
            os << *room << std::endl;
        }
        return os;
    }

    // Mandatory for unordered_map
    int getId() const {
        return id;
    }

    // Mandatory for assigning a room to a house
    void addRoom(const std::shared_ptr<Room>& temp) {
        rooms.push_back(temp);
    }

    ~House() = default;
};
```

**2. Create your CSV files**

*Usually, your executable will be located at* `cmake-build-debug`. *You can create a hierarchy in your project:*

```
project/
├── cmake-build-debug/         # CMake build output directory
├── include/                   # Header files
│   └── CSVParser.h            # CSV parsing utility
├── data/                      # Input or test data
│   └── somedata.csv           # Sample CSV file
└── main.cpp                   # Application entry point
```

*Then, in main.cpp you can access your file through* `../data/somedata.csv`.

- houses.csv (separator: **:** | quote: **"** | header_row: **1** (default))
```
id:address:terrain
1:"742 Evergreen Drive":121
2:"18 Crescent Hollow Lane":213
3:"2051 Ravencliff Road":421
4:"44:Windwhistle Crescent":193
5:"1302 Starling Vale":412
```

- houses.csv (separator: **TAB** | quote: **'** | header_row: **2** (indexing from 1))
```
# This is a CSV!
id	house_id	room_name	capacity	surface
1	1	'Kitchen 2'	4	12
2	1	Bedroom	3	18
3	1	Bedroom	1	9
4	1	Living 	4	20
5	2	Living 	2	15
6	2	Bedroom	2	12
7	2	Bedroom	1	9
8	3	Studio 	2	26
9	4	Studio 	2	32
10	5	Studio 	5	48
```

**3. Use the library as described in *Syntax***
```
// Assuming that the library will be used in main.cpp

#include <CSVParser.h>

// ... previous objects definitions

int main(){

    // The subobject
    // Room(const int id_, const int house_id_, std::string  room_name_, const int capacity_, const float surface_)
    CSVParser<Room, int, int, std::string, int, float> rooms_parser({"Room ID", "House ID", "Room name", "Capacity", "Surface"});
    rooms_parser.setQuote('\'');        // Set the custom quotation symbol
    rooms_parser.setDelimiter('\t');    // Set the custom delimiter

    // Example: Result all_rooms will be of type std::set<std::shared_ptr<Room>>
    const auto all_rooms = rooms_parser.parsePointerObjectsFromFile<std::set>("../tests/rooms.csv");

    // The main object
    // House (const int id_, std::string address_, int surface_)
    CSVParser<House, int, std::string, int> houses_parser({"House ID", "Address", "Terrain"});
    houses_parser.setQuote('\'');        // Set the custom quotation symbol
    houses_parser.setDelimiter(':');     // Set the custom delimiter
    
    // Example: Result all_houses will be of type std::unordered_map<int, House>
    auto all_houses = houses_parser.parseObjectsFromFile<std::unordered_map, int>("../tests/houses.csv");
    
    // Assigning each house their correspondent rooms
    for (const auto& room : all_rooms) {
        // For each object from all_rooms, obtain the external key and assign the room to its correspondent house
        if (auto it = all_houses.find(room->getHouseId()); it != all_houses.end()) {
            it->second.addRoom(room);   // Using . because it->seconds is an object, not a pointer
        }
    }
    
    // Inspecting retrieved objects (inspecting the containers)
    // Requires operator<< overload for both objects. (friend std::ostream & operator<<)
    houses_parser.inspect(all_houses);          // Using inspect because all_houses contains pure objects
    rooms_parser.inspect_pointers(all_rooms);   // Using isnpect_pointers because all_rooms contains std::shared_ptr<Rooms>
}

```

- ## Spotted a 🐛?
Raise an [issue](https://github.com/vlaxcs/CSV-Object-Parser-CPP/issues) and I will review as soon as possible.