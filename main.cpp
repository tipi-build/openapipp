#include <functional>
#include <variant>
#include <iostream>

#include <cx_algorithm.h>

#include <cx_json_parser.h>
#include <cx_json_value.h>

#include <ranges>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <string_view>
#include <tuple>

#include <boost/pfr.hpp>
#include <boost/mp11.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include <pre/cx/key_value_pair.hpp>

#include <pre/json/to_json.hpp>
#include <pre/json/from_json.hpp>

#include <xxhr/xxhr.hpp>


#include "api.json.hpp"

// evaluates to the type returned by a constexpr lambda
template <typename Identifier>
using identifier_type = decltype(std::declval<Identifier>()());

template <class> inline constexpr bool always_false_v = false;

using namespace JSON::literals;

template <auto Start, auto End, auto Inc, class F>
constexpr void constexpr_for(F&& f)
{
    if constexpr (Start < End)
    {
        f(std::integral_constant<decltype(Start), Start>());
        constexpr_for<Start + Inc, End, Inc>(f);
    }
}


template <cx::static_string>
struct named_type {};

template <> 
struct named_type<"integer"> { using type = int; };

template <>
struct named_type<"boolean"> { using type = bool; };


template <>
struct named_type<"string"> { using type = std::string; };

template <>
struct named_type<"array"> { using type = std::vector<std::string>; };



template <cx::static_string S>
using named_type_t = typename named_type<cx::static_shrink_to_fit<S>()>::type;

  consteval bool key_exists(const auto& object, const auto& key) {
    const auto& ext = object.object_storage.get()[object.index].to_Object();
    for (auto i = ext.offset; i < ext.offset + ext.extent; i += 2) {
      const auto& str = object.object_storage.get()[i].to_String();
      cx::static_string k { &object.string_storage.get()[str.offset], str.extent };
      if (k == key) {
        return true;
      }
    } 
    return false;
  }

  consteval auto get_schema_name(const auto& schema_ref) {
      auto segment_begin = schema_ref.begin() + 2; // Skip #/
      auto segment_end = std::find(segment_begin, schema_ref.end(), '/');
      for (; segment_end != schema_ref.end();) {
        cx::static_string path_segment{segment_begin, segment_end};
        segment_begin = segment_end + 1;
        segment_end = std::find(segment_begin + 1, schema_ref.end(), '/');

      } 
      auto last_segment = cx::static_string{segment_begin, segment_end};
      return last_segment;
  }

  consteval auto get_json_by_schema_ref(const auto& schema_ref) {
      JSON::value_proxy deeper_json = JSON::value_proxy{0, tipi::build::openapi_json.object_storage, tipi::build::openapi_json.string_storage}; 
      
      auto segment_begin = schema_ref.begin() + 2; // Skip #/
      auto segment_end = std::find(segment_begin, schema_ref.end(), '/');
      for (; segment_end != schema_ref.end();) {
        cx::static_string path_segment{segment_begin, segment_end};
        deeper_json = deeper_json[path_segment];
        segment_begin = segment_end + 1;
        segment_end = std::find(segment_begin + 1, schema_ref.end(), '/');

      } 
      auto last_segment = cx::static_string{segment_begin, segment_end};
      return deeper_json[last_segment];
  }

  template<cx::static_string path>
  consteval auto create_path_caller()  {
    auto paths = tipi::build::openapi_json["paths"];

    const auto& ext = paths.object_storage.get()[paths.index].to_Object();
    bool notfound = true;
    for (auto i = ext.offset; i < ext.offset + ext.extent; i += 2) {
      const auto& str = paths.object_storage.get()[i].to_String();
      cx::static_string k { &paths.string_storage.get()[str.offset], str.extent };
      if (k == path) {
        auto found_value = paths.object_storage.get()[i+1];//JSON::value_proxy{i+1, ext.object_storage.get(), ext.string_storage.get()};
        //return path_caller_t{found_value.to_String()};
        return JSON::value_proxy{i+1, paths.object_storage.get(), paths.string_storage.get()};
      }
    }
    if (notfound) throw std::runtime_error("Path not found in object");
    return JSON::value_proxy{0, paths.object_storage.get(), paths.string_storage.get()};
  }

  template<cx::static_string operationId>
  consteval auto create_operation_caller()  {
    auto paths = tipi::build::openapi_json["paths"];

    const auto& ext = paths.object_storage.get()[paths.index].to_Object();
    bool notfound = true;
    for (auto i = ext.offset; i < ext.offset + ext.extent; i += 2) {
      const auto& str = paths.object_storage.get()[i].to_String();
      cx::static_string k { &paths.string_storage.get()[str.offset], str.extent };
      if (paths[k]["post"]["operationId"].to_String() == operationId) {
        auto found_value = paths.object_storage.get()[i+1];//JSON::value_proxy{i+1, ext.object_storage.get(), ext.string_storage.get()};
        //return path_caller_t{found_value.to_String()};
        return JSON::value_proxy{i+1, paths.object_storage.get(), paths.string_storage.get()};
      }
    }
    if (notfound) throw std::runtime_error("Path not found in object");
    return JSON::value_proxy{0, paths.object_storage.get(), paths.string_storage.get()};
  }


  consteval auto get_operation_name_for(size_t i)  {
    return tipi::build::openapi_json["paths"][i]["post"]["operationId"].to_String();
  }

  template<cx::static_string operationId>
  consteval auto get_path_uri()  {
    auto paths = tipi::build::openapi_json["paths"];

    const auto& ext = paths.object_storage.get()[paths.index].to_Object();
    bool notfound = true;
    for (auto i = ext.offset; i < ext.offset + ext.extent; i += 2) {
      const auto& str = paths.object_storage.get()[i].to_String();
      cx::static_string k { &paths.string_storage.get()[str.offset], str.extent };
      if (paths[k]["post"]["operationId"].to_String() == operationId) {
        auto found_value = paths.object_storage.get()[i+1];//JSON::value_proxy{i+1, ext.object_storage.get(), ext.string_storage.get()};
        //return path_caller_t{found_value.to_String()};
        return k;
      }
    }
    if (notfound) throw std::runtime_error("Path not found in object");
    return cx::static_string {};
  }

  template <cx::static_string operationId>
  constexpr auto get_arguments_count() {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr ((!path_caller["post"]["requestBody"]["required"].is_Null()) && (path_caller["post"]["requestBody"]["required"].to_Boolean())) {
      constexpr auto schema_ref = path_caller["post"]["requestBody"]["content"]["application/json"]["schema"]["$ref"];


      
      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"].object_Size();
    }
  }

  template <cx::static_string operationId>
  constexpr auto get_argument_type(int pos) {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr ((!path_caller["post"]["requestBody"]["required"].is_Null()) && (path_caller["post"]["requestBody"]["required"].to_Boolean())) {
      constexpr auto schema_ref = path_caller["post"]["requestBody"]["content"]["application/json"]["schema"]["$ref"];

      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"][pos]["type"].to_String();
    }
  }

  template <cx::static_string operationId>
  constexpr auto get_argument_name(int pos) {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr ((!path_caller["post"]["requestBody"]["required"].is_Null()) && (path_caller["post"]["requestBody"]["required"].to_Boolean())) {
      constexpr auto schema_ref = path_caller["post"]["requestBody"]["content"]["application/json"]["schema"]["$ref"];

      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"].object_key(pos);
    }
  }

  template <cx::static_string operationId>
  constexpr auto get_response_properties_count() {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr (!path_caller["post"]["responses"]["200"].is_Null()) {
      constexpr auto schema_ref = path_caller["post"]["responses"]["200"]["content"]["application/json"]["schema"]["$ref"];

      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"].object_Size();
    }
  }

  template <cx::static_string operationId>
  constexpr auto get_response_property_name(size_t pos) {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr (!path_caller["post"]["responses"]["200"].is_Null()) {
      constexpr auto schema_ref = path_caller["post"]["responses"]["200"]["content"]["application/json"]["schema"]["$ref"];

      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"].object_key(pos);
    }
  }

  template <cx::static_string operationId>
  constexpr auto get_response_property_type(size_t pos) {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    if constexpr (!path_caller["post"]["responses"]["200"].is_Null()) {
      constexpr auto schema_ref = path_caller["post"]["responses"]["200"]["content"]["application/json"]["schema"]["$ref"];

      constexpr auto schema = get_json_by_schema_ref(schema_ref.to_String());
      return schema["properties"][pos]["type"].to_String();
    }
  }


  template<cx::static_string operationId, class arg_pos>
  struct expected_arg_for {
    using arg_type = named_type_t<get_argument_type<operationId>(arg_pos::value)>;
    static constexpr cx::static_string arg_name = cx::static_shrink_to_fit<get_argument_name<operationId>(arg_pos::value)>();
    using type = pre::cx::key_value_pair<arg_name, arg_type>;
  };

  template<class operationId, class arg_pos>
  using expected_arg_for_t = typename expected_arg_for<operationId::value, arg_pos>::type;

  namespace mp11 = boost::mp11; 
  template<class operationId>
  using arg_list_count = mp11::mp_iota_c<get_arguments_count<operationId::value>()>;

  template<cx::static_string operationId>
  using arg_list_for = mp11::mp_transform_q<mp11::mp_bind<expected_arg_for_t, pre::cx::tstring<operationId>, mp11::_1>, arg_list_count<pre::cx::tstring<operationId>>>;

  template <cx::static_string operationId>
  using arg_list_as_tuple_for = mp11::mp_rename<arg_list_for<operationId>, std::tuple >;






  template<cx::static_string operationId, class arg_pos>
  struct expected_response_for {
    using prop_type = named_type_t<get_response_property_type<operationId>(arg_pos::value)>;
    static constexpr cx::static_string prop_name = cx::static_shrink_to_fit<get_response_property_name<operationId>(arg_pos::value)>();
    using type = pre::cx::key_value_pair<prop_name, prop_type>;
  };

  template<class operationId, class arg_pos>
  using expected_response_for_t = typename expected_response_for<operationId::value, arg_pos>::type;

  namespace mp11 = boost::mp11; 
  template<class operationId>
  using response_list_count = mp11::mp_iota_c<get_response_properties_count<operationId::value>()>;

  template<cx::static_string operationId>
  using response_list_for = mp11::mp_transform_q<mp11::mp_bind<expected_response_for_t, pre::cx::tstring<operationId>, mp11::_1>, response_list_count<pre::cx::tstring<operationId>>>;

  template <cx::static_string operationId>
  using response_list_as_tuple_for = mp11::mp_rename<response_list_for<operationId>, std::tuple >;



template <class TOpenAPIJSON>
struct client_t {
  //static constexpr TOpenAPIJSON config =tipi::build::openapi_json ;
  constexpr client_t() {}

  template <cx::static_string path, class... Args>
  constexpr auto call(Args&&... args) const {
    constexpr auto path_caller = create_path_caller<path>();
    static_assert(!path_caller.is_Null());
    std::cout << "Calling : " << path_caller["post"]["summary"].to_String().c_str() << "\n";
  }

  template <cx::static_string operationId, pre::json::detail::requirements::not_tuple_like T>
  auto call_operation(const T& requestBody) const {
    auto t = boost::pfr::structure_to_tuple(requestBody);
    return call_operation<operationId>(t);
  }

  template <cx::static_string operationId>
  auto call_operation(const arg_list_as_tuple_for<operationId>& requestBody) const {
    constexpr auto path_caller = create_operation_caller<operationId>();
    static_assert(!path_caller.is_Null());

    auto body = pre::json::to_json(requestBody).dump(2);
    std::cout << body << std::endl;

    auto response = response_list_as_tuple_for<operationId>();
    constexpr auto route_path_for_operation = get_path_uri<operationId>();
    using namespace xxhr;
    POST("https://petstore3.swagger.io/api/v3"s + route_path_for_operation.c_str(), 
      Header {
        {"Accept", "application/json"},
        {"Content-Type", "application/json"}
      },
      Body { body },
      on_response = [&](auto&& resp) {
        std::cout << "Reply : " << resp.status_code << ": " << resp.text << std::endl;
        if (resp.status_code == 200) {
          response = pre::json::from_json<decltype(response)>(resp.text);
        } 
      });

    return response;
  }

};


namespace poor_person_metaclass {

  template <cx::static_string operationId>
  inline void generate_operation() {

    constexpr auto path_caller = create_operation_caller<operationId>();

    constexpr auto response_struct_name = get_schema_name(path_caller["post"]["responses"]["200"]["content"]["application/json"]["schema"]["$ref"].to_String());
    constexpr auto POSTS = path_caller["post"];
    if constexpr(!key_exists(POSTS, "requestBody")) {
      std::cout << "inline " << response_struct_name.c_str() << " "<< operationId.c_str() << " () {" << "\n";
      std::cout << "  " << "client.call_operation<\"" << operationId.c_str() << "\">( );" << "\n";
      std::cout << "}"<< "\n";

      std::cout << "struct " << response_struct_name.c_str() << " {" << "\n";
      for (size_t i = 0; i < get_response_properties_count<operationId>(); ++i) {
        std::cout << "  " << "pre_json_key(" <<   get_response_property_type<operationId>(i).c_str() << ", " << get_response_property_name<operationId>(i).c_str() << " );" << "\n";
      }
      std::cout << "\n" << "};" << "\n";
    } else {
      std::cout << "}"<< "\n";
      constexpr auto schema_ref = path_caller["post"]["requestBody"]["content"]["application/json"]["schema"]["$ref"];
      constexpr auto request_struct_name = get_schema_name(schema_ref.to_String());

      std::cout << "struct " << request_struct_name.c_str() << " {" << "\n";
      for (size_t i = 0; i < get_arguments_count<operationId>(); ++i) {
        std::cout << "  " << "pre_json_key( named_type<\"" <<   get_argument_type<operationId>(i).c_str() << "\">::type, " << get_argument_name<operationId>(i).c_str() << " );" << "\n";
      }
      std::cout << "\n" << "};" << "\n";

      std::cout << "struct " << response_struct_name.c_str() << " {" << "\n";
      for (size_t i = 0; i < get_response_properties_count<operationId>(); ++i) {
        std::cout << "  " << "pre_json_key( named_type<\"" <<   get_response_property_type<operationId>(i).c_str() << "\">::type, " << get_response_property_name<operationId>(i).c_str() << " );" << "\n";
      }
      std::cout << "\n" << "};" << "\n";

      std::cout << "inline " << response_struct_name.c_str() << " " << operationId.c_str() << " ( const " << request_struct_name.c_str() << "& requestBody) {" << "\n";
      std::cout << "  " << "client.call_operation<\"" << operationId.c_str() << "\">( requestBody );" << "\n";
      std::cout << "}"<< "\n";


    }
  }

  template<class idx_op>
  struct generate_operation_t {
    generate_operation_t() { 
      constexpr auto op = get_operation_name_for(idx_op::value);
      generate_operation<cx::static_shrink_to_fit<get_operation_name_for(idx_op::value)>()>();
      
    }
  };

  consteval std::size_t available_paths() { return tipi::build::openapi_json["paths"].object_Size(); }

  using operations_index_seq = mp11::mp_iota_c<available_paths()>;

  using all_operations = mp11::mp_transform<generate_operation_t, operations_index_seq>;

}

// GENERATED code mp11::mp_rename<poor_person_metaclass::all_operations, mp11::mp_inherit> print_code{};
struct Pet {
  pre_json_key( named_type<"integer">::type, id );
  pre_json_key( named_type<"string">::type, name );
  pre_json_key( named_type<"array">::type, photoUrls );

};

inline Pet addPet ( const Pet& requestBody) {
  constexpr auto client = client_t<decltype(tipi::build::openapi_json)>{};
  Pet returnval;
  std::tuple tupled {boost::pfr::structure_tie(returnval)};
  auto response = client.call_operation<"addPet">( requestBody );
  tupled = response;
  return returnval;
}
// GENERATED

int main(int argc, char** argv) {
  using namespace std::literals;

  constexpr auto client = client_t<decltype(tipi::build::openapi_json)>{};

  using pre::cx::key_value_pair;

  auto reply = client.call_operation<"addPet">(std::make_tuple(
    key_value_pair<"id" , int>  {43},
    key_value_pair<"name" , std::string>  {"Haskell"s},
    key_value_pair<"photoUrls" , std::vector<std::string>>  { {"little_cat.png"s} }
  ));

  auto second_reply = addPet(Pet{{43}, {"Haskell junior"s}, {{"some_cat.png"}} }); 

  // Generate code mp11::mp_rename<poor_person_metaclass::all_operations, mp11::mp_inherit> print_code{};
}
