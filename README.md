# OpenApi++ : openapipp
This is a proof of concept, currently under active work to become the best OpenAPI implementation for C++.

It allows compile time C++ code generation for an OpenAPI configuration.


## To compile the project and pull dependencies run 
Get a free tipi by registering to [tipi.build](https://tipi.build)
```
tipi . -t macos-cxx20
```  

## CppCon 2021 Talk
Featured in the [CppCon 2021 Talks : Our Adventures With REST API in C++ : Making it Easy](https://cppcon2021.sched.com/event/nvCg/our-adventures-with-rest-api-in-c-making-it-easy?iframe=no).

  * Slides available here : [CppCon 2021 Talks : Our Adventures With REST API in C++ : Making it Easy](./tipi-CppCon2021-OurAdventureWithRESTApiInCpp.pdf)



## OpenApi JSON Compile Time client generation
Add the content of the [petstore openapi.json](https://petstore3.swagger.io/) to a C++ constexpr variable prefixed by `_cx_json`.

```cpp
  static constexpr auto openapi_json = R"x( 
  {
    "openapi": "3.0.2",
    "info": {
      "title": "Swagger Petstore - OpenAPI 3.0",
      "description": "This is a sample Pet Store Server.",
      "termsOfService": "http://swagger.io/terms/",
      "contact": {
        "email": "apiteam@swagger.io"
      },
      "license": {
        "name": "Apache 2.0",
        "url": "http://www.apache.org/licenses/LICENSE-2.0.html"
      },
      "version": "1.0.6"
      
    }
    ...
    ...
    ...
    ...
  })x"_cx_json

  constexpr auto client = client_t<decltype(openapi_json)>{};
  using pre::cx::key_value_pair;

  auto reply = client.call_operation<"addPet">(std::make_tuple(
    key_value_pair<"id" , int>  {43},
    key_value_pair<"name" , std::string>  {"Haskell"s},
    key_value_pair<"photoUrls" , std::vector<std::string>>  { {"little_cat.png"s} }
  ));
```

### Generate "C++ Typings" for autocompletion support
```cpp
mp11::mp_rename<poor_person_metaclass::all_operations, mp11::mp_inherit> print_code{};
```

This will print the code to `#include <>` on a later build, that allows you then to use this syntax for the `HTTP POST /Pet` remote call: 
```cpp
auto second_reply = addPet(Pet{{43}, {"Haskell junior"s}, {{"some_cat.png"}} }); 
```

`
## Libraries used in this work
  * [Boost.PFR](https://github.com/boostorg/pfr) : Struct to tuple transformation
  * [cpp-pre/json feature branche compile-time-key-value-pairs](https://github.com/cpp-pre/json/tree/feature/compile-time-key-value-pairs): Automagic JSON generation Compile time key value pairs
  * [constexpr_all_the_things fork with C++20 compile time strings]("https://github.com/tipi-build/constexpr_all_the_things")
  * [nxxm/xxhr](https://nxxm.github.io/xxhr) : HTTP requests client library