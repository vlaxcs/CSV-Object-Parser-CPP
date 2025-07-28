# CSV Object Parser C++ Library

***The first ever*** lightweight and header-only C++ library that parses CSV files and maps each row to a strongly-typed C++ object.\
It provides a simple and efficient way to deserialize structured CSV data into containers of custom objects.

**Be careful! CSV Object Parser library is limited to the functionalities described at **Syntax**, but should work properly if you follow all steps.**

> Launch date: 27.07.2025 \
> Last update: 26.07.2025


- ## Requirements

Compatible with C++20 and higher.
Used libraries: 
**[utility]()**, 
**[vector]()**, 
**[string]()**, 
**[memory]()**, 
**[unordered_map]()**, 
**[algorithm]()**, 
**[fstream]()**, 
**[sstream]()**, 
**[exception]()**, 
**[format]()**,
**[set]()**

- ## Installation

1. Open your C++ project with CLion.
2. Create a `include` directory
3. In CMakeLists.txt, add `target_include_directories(project_name PRIVATE include)` and replace `project_name` with your project's folder name.
4. Clone the repository: `git clone https://github.com/vlaxcs/CSV-Object-Parser-CPP`
5. From the cloned repository, move `CSVParser.h` into your `include` folder.
6. In the file you want to process your objects, include the library using `#include <CSVParser.h>`.


- ## Syntax

### Template instantiation

There are 2 ways CSV Object Parser can approach reaching data from the CSV file.

1. Assumes that all arguments and all CSV cells (except header) can be converted to a **UniqueType**.
   - Signature:\
   `CSVParser<Object, UniqueType> all_objects;`
  
   - Example:\
   `CSVParser<Rating, float> all_ratings;`

2. Needs to know all constructor's arguments' type.
   - **TypeK** belongs to **K-th** argument of Object's constructor, only if Object's constructor has **N** (1, 2, ... K ..., N - 1, N) arguments.
   - Signature:\
     `CSVParser<Object, Type1, Type2, Type3 ... TypeN> all_objects;`

   - Example:\
     `CSVParser<Object, int, float, string, AnotherObject> all_objects;`
        - **Works only** if the one of the object's constructors is defined as `Object(int _, float _, string _, *AnotherObject* _)`.
        - If another object is needed as argument (e.g. *AnotherObject*), this object should have a proper overload on **>>operator**.

- Illegal: `CSVParser<Object>` with no type given. 
- Be aware of this example: `CSVParser<int, float>` will build **int objects**, assuming that all your CSV cells can be read as **float**.

### Parsing object instantiation

1. No header preferences. Will use CSV file's header and will establish the delimiter by the maximum frequency on first 2 rows of your document.\
   - Constructor signature (default): `CSVParser()`
   - `CSVParser<...> all_objects;`


2. Using a custom header. Column names are not very important, but the length of header list should match the header length from CSV file.
   - Constructor signature: `CSVParser(const std::vector<std::string> &header_)`
   - Example: `CSVParser<...> all_objects({"First column", "Second column"});`
   - For these column names, CSV's file header must look like `text1,text2`, delimiter is not important here.

### Parsing object customization

1. Set the delimiter symbol.
   - `all_objects.setDelimiter(const char symbol)`

2. Set the quotation symbol.
   - `all_objects.setQuote(const char symbol)`

3. Set the header row index (Default indexed from 1. The custom header row index must start from 1).
   - `all_objects.setHeaderRow(const int)`
   

### Parsing from a file

1. The result is a vector of objects.

2. *(Not published yet). The result is an unordered_map<int, Object>. Key: ID | Value: Object*

3. *(Not published yet). The result is a set.*

### Header properties
  - Delimiter set:
     - No header preference -> Sets the header separated by the delimiter.
     - Header preference    -> Sets the header separated by the delimiter only if the lengths are equal ***or Throws on size mismatch***.

  - Delimiter not set:
    - No header preference  -> Tries to find headers using default CSV delimiters. Sets the header of maximum matched length with next row's length (first row of values).
    - Header preference     -> Sets the header separated by the one of the default delimiters only if the lengths of file's header and preferred header are equal ***or Throws on size mismatch***.

- ## Benchmarks

| Object | Operation                                      | Count   | Time |
|--------|------------------------------------------------|---------|------|
| Object(int, string, string, float) | CSVParser construction + parseObjectFromFile() | 200.000 | 1.1s  |  
| Object(int, string, string, float) | CSVParser construction + parseObjectFromFile()  | 700.000 | 4.0s  |

- ## How to use the library (Step by step example)

**1. Create a public constructor with all attributes you want to instance with CSV file's content.**

```
// Case 1 - All constructor's arguments must be the same type and the constructor's arity must be the maximum.

class Course{
private:
    std::string name;
    std::string teacher_fullname;
    std::string website_url;

public:
    Course(const std::string& name_, const std::string& teacher_fullname, const std::string& website_url_)
        : name(std::move(name_)), teacher_fullname(std::move(teacher_fullname)), website_url(std::move(website_url_)) {}
}


// Case 2 - Constructor's arguments can be of different types.

class Student{
private:
    std::string first_name;
    int study_year;
    float annual_average;
    
public:
    Student(const std::string& first_name_, int study_year_, float annual_average_)
        : first_name(first_name_), study_year(study_year_), annual_average(annual_average_) {}
}
```

**2. Create your CSV files**

- courses.csv
```
c1,c2,c3
Data Structures,C.Smith,demo.com/ds
Data Bases,P.Smith,demo.com/db
Computer Architecture,P.Smith,demo.com/ca
```

- students.csv
```
# This is a CSV!
c1~c2~c3
Carlos~1~9.82
Sebastian~2~7.14
Viktor~2~8.38
```

**3. Use the library as described in *Syntax***
```
// Assuming that the library will be used in main.cpp

#include <CSVParser.h>

// ... previous objects definitions

int main(){

    // Example 1 - Course - Unique type
    std::string csv_with_courses("courses.csv");    
    
    CSVParser<Student, std::string> all_courses_parser;
    auto courses = all_courses_parser.parseObjectFromFile<std::vector>(../tests/csv_with_courses);      // Will create a vector of objects
    
    // Example 2 - Students - Multiple types
    std::string csv_with_students("students.csv");
   
    CSVParser<Student, std::string, int, float> all_students_parser;
    all_students_parser.setDelimiter('~');          // Delimiter is '~'
    all_students_parser.setHeaderRow(2);            // Indexed from 1 (Skip '# This is a CSV!')
    auto students = all_students_parser.parseObjectFromFile<std::set>(../tests/csv_with_students);       // Will create a set of objects
}

```

- ## Spotted a 🐛?
Raise an issue and I will review as soon as possible.
