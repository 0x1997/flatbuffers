/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <tuple>

#include "src/compiler/python_generator.h"

namespace grpc_python_generator {

namespace {

//////////////////////////////////
// BEGIN FORMATTING BOILERPLATE //
//////////////////////////////////


// Provides RAII indentation handling. Use as:
// {
//   IndentScope raii_my_indent_var_name_here(my_py_printer);
//   // constructor indented my_py_printer
//   ...
//   // destructor called at end of scope, un-indenting my_py_printer
// }

class IndentScope {
 public:
  explicit IndentScope(grpc_generator::Printer* printer) : printer_(printer) {
    printer_->Indent();
  }

  ~IndentScope() { printer_->Outdent(); }

 private:
  grpc_generator::Printer* printer_;
};

////////////////////////////////
// END FORMATTING BOILERPLATE //
////////////////////////////////

void PrintBetaServicer(const grpc_generator::Service *service,
                             grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;

  vars["Service"] = service->name();
  out->Print("\n\n");
  out->Print(vars, "class Beta$Service$Servicer(object):\n");
  {
    IndentScope raii_class_indent(out);
    for (int i = 0; i < service->method_count(); ++i) {
      auto meth = service->method(i);
      grpc::string arg_name =
          meth.get()->ClientOnlyStreaming() ? "request_iterator" : "request";
          vars["Method"] = meth.get()->name();
          vars["ArgName"] = arg_name;
      out->Print(vars, "def $Method$(self, $ArgName$, context):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("context.code(beta_interfaces.StatusCode.UNIMPLEMENTED)\n");
      }
    }
  }
}

void PrintBetaStub(const grpc_generator::Service *service,
                         grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["Service"] = service->name();
  out->Print("\n\n");
  out->Print(vars, "class Beta$Service$Stub(object):\n");
  {
    IndentScope raii_class_indent(out);
    for (int i = 0; i < service->method_count(); ++i) {
      auto meth = service->method(i);
      grpc::string arg_name =
          meth.get()->ClientOnlyStreaming() ? "request_iterator" : "request";
      std::map<grpc::string, grpc::string> methdict;
      methdict["Method"] = meth.get()->name();
      methdict["ArgName"] = arg_name;
      out->Print(methdict,
                 "def $Method$(self, $ArgName$, timeout, metadata=None, "
                 "with_call=False, protocol_options=None):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("raise NotImplementedError()\n");
      }
      if (!meth.get()->ServerOnlyStreaming()) {
        out->Print(methdict, "$Method$.future = None\n");
      }
    }
  }
}

void PrintBetaServerFactory(const grpc::string &package_qualified_service_name,
                            const grpc_generator::Service *service,
                                  grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["Service"] = service->name();
  out->Print("\n\n");
  out->Print(vars,
      "def beta_create_$Service$_server(servicer, pool=None, "
      "pool_size=None, default_timeout=None, maximum_timeout=None):\n");
  {
    IndentScope raii_create_server_indent(out);
    std::map<grpc::string, grpc::string> method_implementation_constructors;
    std::map<grpc::string, grpc::string> input_message_modules_and_classes;
    std::map<grpc::string, grpc::string> output_message_modules_and_classes;
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      const grpc::string method_implementation_constructor =
          grpc::string(method.get()->ClientOnlyStreaming() ? "stream_" : "unary_") +
          grpc::string(method.get()->ServerOnlyStreaming() ? "stream_" : "unary_") +
          "inline";
      grpc::string input_message_module_and_class = method.get()->request_name();
      grpc::string output_message_module_and_class = method.get()->response_name();
      method_implementation_constructors.insert(
          std::make_pair(method.get()->name(), method_implementation_constructor));
      input_message_modules_and_classes.insert(
          std::make_pair(method.get()->name(), input_message_module_and_class));
      output_message_modules_and_classes.insert(
          std::make_pair(method.get()->name(), output_message_module_and_class));
    }
    out->Print("request_deserializers = {\n");
    for (auto name_and_input_module_class_pair =
             input_message_modules_and_classes.begin();
         name_and_input_module_class_pair !=
         input_message_modules_and_classes.end();
         name_and_input_module_class_pair++) {
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      vars["MethodName"] = name_and_input_module_class_pair->first;
      vars["InputTypeModuleAndClass"] = name_and_input_module_class_pair->second;
      IndentScope raii_indent(out);
      out->Print(vars,
          "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
          "$InputTypeModuleAndClass$.FromString,\n");
    }
    out->Print("}\n");
    out->Print("response_serializers = {\n");
    for (auto name_and_output_module_class_pair =
             output_message_modules_and_classes.begin();
         name_and_output_module_class_pair !=
         output_message_modules_and_classes.end();
         name_and_output_module_class_pair++) {
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      vars["MethodName"] = name_and_output_module_class_pair->first;
      vars["OutputTypeModuleAndClass"] = name_and_output_module_class_pair->second;
      IndentScope raii_indent(out);
      out->Print(vars,
          "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
          "$OutputTypeModuleAndClass$.SerializeToString,\n");
    }
    out->Print("}\n");
    out->Print("method_implementations = {\n");
    for (auto name_and_implementation_constructor =
             method_implementation_constructors.begin();
         name_and_implementation_constructor !=
         method_implementation_constructors.end();
         name_and_implementation_constructor++) {
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      vars["Method"] = name_and_implementation_constructor->first;
      vars["Constructor"] = name_and_implementation_constructor->second;
      IndentScope raii_descriptions_indent(out);
      const grpc::string method_name =
          name_and_implementation_constructor->first;
      out->Print(vars,
          "(\'$PackageQualifiedServiceName$\', \'$Method$\'): "
          "face_utilities.$Constructor$(servicer.$Method$),\n");
    }
    out->Print("}\n");
    out->Print(
        "server_options = beta_implementations.server_options("
        "request_deserializers=request_deserializers, "
        "response_serializers=response_serializers, "
        "thread_pool=pool, thread_pool_size=pool_size, "
        "default_timeout=default_timeout, "
        "maximum_timeout=maximum_timeout)\n");
    out->Print(
        "return beta_implementations.server(method_implementations, "
        "options=server_options)\n");
  }
}

void PrintBetaStubFactory(const grpc::string &package_qualified_service_name,
                          const grpc_generator::Service *service,
                                grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["Service"] = service->name();
  out->Print("\n\n");
  out->Print(vars,
             "def beta_create_$Service$_stub(channel, host=None,"
             " metadata_transformer=None, pool=None, pool_size=None):\n");
  {
    IndentScope raii_create_server_indent(out);
    std::map<grpc::string, grpc::string> method_cardinalities;
    std::map<grpc::string, grpc::string> input_message_modules_and_classes;
    std::map<grpc::string, grpc::string> output_message_modules_and_classes;
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      const grpc::string method_cardinality =
          grpc::string(method.get()->ClientOnlyStreaming() ? "STREAM" : "UNARY") + "_" +
          grpc::string(method.get()->ServerOnlyStreaming() ? "STREAM" : "UNARY");
      grpc::string input_message_module_and_class = method.get()->request_name();
      grpc::string output_message_module_and_class = method.get()->response_name();
      method_cardinalities.insert(
          std::make_pair(method.get()->name(), method_cardinality));
      input_message_modules_and_classes.insert(
          std::make_pair(method.get()->name(), input_message_module_and_class));
      output_message_modules_and_classes.insert(
          std::make_pair(method.get()->name(), output_message_module_and_class));
    }
    out->Print("request_serializers = {\n");
    for (auto name_and_input_module_class_pair =
             input_message_modules_and_classes.begin();
         name_and_input_module_class_pair !=
         input_message_modules_and_classes.end();
         name_and_input_module_class_pair++) {
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      vars["MethodName"] = name_and_input_module_class_pair->first;
      vars["InputTypeModuleAndClass"] = name_and_input_module_class_pair->second;
      IndentScope raii_indent(out);
      out->Print(vars,
          "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
          "$InputTypeModuleAndClass$.SerializeToString,\n");
    }
    out->Print("}\n");
    out->Print("response_deserializers = {\n");
    for (auto name_and_output_module_class_pair =
             output_message_modules_and_classes.begin();
         name_and_output_module_class_pair !=
         output_message_modules_and_classes.end();
         name_and_output_module_class_pair++) {
      IndentScope raii_indent(out);
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      vars["MethodName"] = name_and_output_module_class_pair->first;
      vars["OutputTypeModuleAndClass"] = name_and_output_module_class_pair->second;
      out->Print(vars,
          "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
          "$OutputTypeModuleAndClass$.FromString,\n");
    }
    out->Print("}\n");
    out->Print("cardinalities = {\n");
    for (auto name_and_cardinality = method_cardinalities.begin();
         name_and_cardinality != method_cardinalities.end();
         name_and_cardinality++) {
      vars["Method"] = name_and_cardinality->first;
      vars["Cardinality"] = name_and_cardinality->second;
      IndentScope raii_descriptions_indent(out);
      out->Print(vars, "\'$Method$\': cardinality.Cardinality.$Cardinality$,\n");
    }
    out->Print("}\n");
    out->Print(
        "stub_options = beta_implementations.stub_options("
        "host=host, metadata_transformer=metadata_transformer, "
        "request_serializers=request_serializers, "
        "response_deserializers=response_deserializers, "
        "thread_pool=pool, thread_pool_size=pool_size)\n");
    vars["PackageQualifiedServiceName"] = package_qualified_service_name;
    out->Print(vars,
        "return beta_implementations.dynamic_stub(channel, "
        "\'$PackageQualifiedServiceName$\', "
        "cardinalities, options=stub_options)\n");
  }
}

void PrintStub(const grpc::string &package_qualified_service_name,
               const grpc_generator::Service *service,
                     grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  out->Print("\n\n");
  vars["Service"] = service->name();
  out->Print(vars, "class $Service$Stub(object):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print("\n");
    out->Print("def __init__(self, channel):\n");
    {
      IndentScope raii_init_indent(out);
      out->Print("\"\"\"Constructor.\n");
      out->Print("\n");
      out->Print("Args:\n");
      {
        IndentScope raii_args_indent(out);
        out->Print("channel: A grpc.Channel.\n");
      }
      out->Print("\"\"\"\n");
      for (int i = 0; i < service->method_count(); ++i) {
        auto method = service->method(i);
        auto multi_callable_constructor =
            grpc::string(method.get()->ClientOnlyStreaming() ? "stream" : "unary") +
            "_" + grpc::string(method.get()->ServerOnlyStreaming() ? "stream" : "unary");
        grpc::string request_module_and_class = method.get()->request_name();
        grpc::string response_module_and_class = method.get()->response_name();
        vars["Method"] = method.get()->name();
        vars["MultiCallableConstructor"] = multi_callable_constructor;
        out->Print(vars, "self.$Method$ = channel.$MultiCallableConstructor$(\n");
        {
          IndentScope raii_first_attribute_indent(out);
          IndentScope raii_second_attribute_indent(out);
          vars["PackageQualifiedService"] = package_qualified_service_name;
          vars["Method"] = method.get()->name();
          vars["RequestModuleAndClass"] = request_module_and_class;
          vars["ResponseModuleAndClass"] = response_module_and_class;
          out->Print(vars, "'/$PackageQualifiedService$/$Method$',\n");
          out->Print(vars,
              "request_serializer=$RequestModuleAndClass$.SerializeToString,\n");
          out->Print(vars,
              "response_deserializer=$ResponseModuleAndClass$.FromString,\n");
          out->Print(")\n");
        }
      }
    }
  }
}

void PrintServicer(const grpc_generator::Service *service,
                         grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["Service"] = service->name();
  out->Print("\n\n");
  out->Print(vars, "class $Service$Servicer(object):\n");
  {
    IndentScope raii_class_indent(out);
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      grpc::string arg_name =
          method.get()->ClientOnlyStreaming() ? "request_iterator" : "request";
      out->Print("\n");
      vars["Method"] = method.get()->name();
      vars["ArgName"] = arg_name;
      out->Print(vars, "def $Method$(self, $ArgName$, context):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("context.set_code(grpc.StatusCode.UNIMPLEMENTED)\n");
        out->Print("context.set_details('Method not implemented!')\n");
        out->Print("raise NotImplementedError('Method not implemented!')\n");
      }
    }
  }
}

void PrintAddServicerToServer(
    const grpc::string &package_qualified_service_name,
    const grpc_generator::Service *service,
          grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  out->Print("\n\n");
  vars["Service"] = service->name();
  out->Print(vars, "def add_$Service$Servicer_to_server(servicer, server):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print("rpc_method_handlers = {\n");
    {
      IndentScope raii_dict_first_indent(out);
      IndentScope raii_dict_second_indent(out);
      for (int i = 0; i < service->method_count(); ++i) {
        auto method = service->method(i);
        auto method_handler_constructor =
            grpc::string(method.get()->ClientOnlyStreaming() ? "stream" : "unary") +
            "_" +
            grpc::string(method.get()->ServerOnlyStreaming() ? "stream" : "unary") +
            "_rpc_method_handler";
        grpc::string request_module_and_class = method.get()->request_name();
        grpc::string response_module_and_class = method.get()->response_name();
        vars["Method"] = method.get()->name();
        vars["MethodHandlerConstructor"] = method_handler_constructor;
        out->Print(vars, "'$Method$': grpc.$MethodHandlerConstructor$(\n");
        {
          IndentScope raii_call_first_indent(out);
          IndentScope raii_call_second_indent(out);
          vars["Method"] = method.get()->name();
          vars["RequestModuleAndClass"] = request_module_and_class;
          vars["ResponseModuleAndClass"] = response_module_and_class;
          out->Print(vars, "servicer.$Method$,\n");
          out->Print(vars,
              "request_deserializer=$RequestModuleAndClass$.FromString,\n");
          out->Print(vars,
              "response_serializer=$ResponseModuleAndClass$.SerializeToString,"
              "\n");
        }
        out->Print("),\n");
      }
    }
    out->Print("}\n");
    out->Print("generic_handler = grpc.method_handlers_generic_handler(\n");
    {
      IndentScope raii_call_first_indent(out);
      IndentScope raii_call_second_indent(out);
      vars["PackageQualifiedServiceName"] = package_qualified_service_name;
      out->Print(vars, "'$PackageQualifiedServiceName$', rpc_method_handlers)\n");
    }
    out->Print("server.add_generic_rpc_handlers((generic_handler,))\n");
  }
}

void PrintPreamble(const GeneratorConfiguration &config,
                         grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["Package"] = config.grpc_package_root;
  out->Print(vars, "import $Package$\n");
  vars["Package"] = config.beta_package_root;
  out->Print(vars, "from $Package$ import implementations as beta_implementations\n");
  out->Print(vars, "from $Package$ import interfaces as beta_interfaces\n");
  out->Print("from grpc.framework.common import cardinality\n");
  out->Print(
      "from grpc.framework.interfaces.face import utilities as "
      "face_utilities\n");
}

}  // namespace

grpc::string GetServices(grpc_generator::File *file,
                         const GeneratorConfiguration &config) {
  grpc::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto out = file->CreatePrinter(&output);
    PrintPreamble(config, out.get());
    auto package = file->package();
    if (!package.empty()) {
      package = package.append(".");
    }
    for (int i = 0; i < file->service_count(); ++i) {
      auto service = file->service(i);
      auto package_qualified_service_name = package + service.get()->name();
      PrintStub(package_qualified_service_name, service.get(), out.get());
      PrintServicer(service.get(), out.get());
      PrintAddServicerToServer(package_qualified_service_name, service.get(),
                                                    out.get());
      PrintBetaServicer(service.get(), out.get());
      PrintBetaStub(service.get(), out.get());
      PrintBetaServerFactory(package_qualified_service_name, service.get(),
                                                  out.get());
      PrintBetaStubFactory(package_qualified_service_name, service.get(),
                                                out.get());
    }
  }
  return output;
}

}  // namespace grpc_python_generator
