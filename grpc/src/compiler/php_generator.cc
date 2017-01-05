/*
 *
 * Copyright 2016, Google Inc.
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

#include "src/compiler/php_generator.h"
#include "src/compiler/generator_helpers.h"

namespace grpc_php_generator {
namespace {

grpc::string MessageIdentifierName(const grpc::string &name) {
  std::vector<grpc::string> tokens = grpc_generator::tokenize(name, ".");
  std::ostringstream oss;
  for (unsigned int i = 0; i < tokens.size(); i++) {
    if (i)
        oss << '\\';
    oss << tokens[i];
  }
  return oss.str();
}

void PrintMethod(grpc_generator::File *file, const grpc::string &service_name,
                 const grpc_generator::Method *method,
                 grpc_generator::Printer *out) {
  const grpc::string input_type = method->input_name();
  const grpc::string output_type = method->output_name();
  std::map<grpc::string, grpc::string> vars;
  vars["service_name"] = service_name;
  vars["name"] = method->name();
  vars["input_type_id"] = input_type;
  vars["output_type_id"] = output_type;
  vars["package"] = MessageIdentifierName(file->package());
  out->Print("/**\n");
  if (method->BidiStreaming()) {
    out->Print(vars,
               " * @param array $$metadata metadata\n"
               " * @param array $$options call options\n */\n"
               "public function $name$($$metadata = [], "
               "$$options = []) {\n");
    out->Indent();
    out->Print(vars,
               "return $$this->_bidiRequest("
               "'/$service_name$/$name$',\n");
    out->Indent();
    out->Print(vars,
               "'\\$package$\\$output_type_id$::getRootAs$output_type_id$FromBytes',\n");
    out->Print(vars,
               "$$metadata, $$options);\n");
  } else if (method->ClientOnlyStreaming()) {
    out->Print(vars,
               " * @param array $$metadata metadata\n"
               " * @param array $$options call options\n */\n"
               "public function $name$($$metadata = [], "
               "$$options = []) {\n");
    out->Indent();
    out->Print(vars,
               "return $$this->_clientStreamRequest("
               "'/$service_name$/$name$',\n");

    out->Print(vars,
               "'\\$package$\\$output_type_id$::getRootAs$output_type_id$FromBytes',\n");
    out->Print(vars,
               "$$metadata, $$options);\n");
  } else if (method->ServerOnlyStreaming()) {
    out->Print(vars,
               " * @param $input_type_id$ $$argument input argument\n"
               " * @param array $$metadata metadata\n"
               " * @param array $$options call options\n */\n"
               "public function $name$(\\Google\\FlatBuffers\\FlatBufferBuilder $$argument,\n"
               "  $$metadata = [], $$options = []) {\n");
    out->Indent();
    out->Print(vars,
               "return $$this->_serverStreamRequest("
               "'/$service_name$/$name$',\n"
               "$$argument,\n");

    out->Print(vars,
               "'\\$package$\\$output_type_id$::getRootAs$output_type_id$FromBytes',\n");
    out->Print(vars,
               "$$metadata, $$options);\n");
  } else {
    out->Print(vars,
               " * @param $input_type_id$ $$argument input argument\n"
               " * @param array $$metadata metadata\n"
               " * @param array $$options call options\n */\n"
               "public function $name$(\\Google\\FlatBuffers\\FlatBufferBuilder $$argument,"
               " $$metadata = [], $$options = []) {\n");
    out->Indent();
    out->Print(vars,
               "return $$this->_simpleRequest('/$service_name$/$name$',\n"
               "$$argument,\n");
    out->Print("'data',\n");
    out->Print(vars,
               "'\\$package$\\$output_type_id$::getRootAs$output_type_id$FromBytes',\n");
    out->Print(vars,
               "$$metadata, $$options);\n");
  }
  out->Outdent();
  out->Print("}\n\n");
}

// Prints out the service descriptor object
void PrintService(grpc_generator::File *file,
                  const grpc_generator::Service *service,
                  grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["name"] = service->name();
  out->Print(vars, "class $name$Client extends \\Grpc\\BaseStub {\n\n");
  out->Indent();
  out->Print(
      "/**\n * @param string $hostname hostname\n"
      " * @param array $opts channel options\n"
      " * @param Grpc\\Channel $channel (optional) re-use channel "
      "object\n */\n"
      "public function __construct($hostname, $opts, "
      "$channel = null) {\n");
  out->Indent();
  out->Print("parent::__construct($hostname, $opts, $channel);\n");
  out->Outdent();
  out->Print("}\n\n");
  
  std::string package = file->package();
  if (!package.empty())
      package.append(".");
  
  for (int i = 0; i < service->method_count(); i++) {
    PrintMethod(file, package + service->name(), service->method(i).get(), out);
  }
  out->Outdent();
  out->Print("}\n\n");
}

void PrintServices(grpc_generator::File *file, grpc_generator::Printer *out) {
  std::map<grpc::string, grpc::string> vars;
  vars["package"] = MessageIdentifierName(file->package());
  out->Print(vars, "namespace $package$;\n\n");
  out->Indent();
  for (int i = 0; i < file->service_count(); i++) {
    PrintService(file, file->service(i).get(), out);
  }
  out->Outdent();
}
}

grpc::string GetServices(grpc_generator::File *file) {
  grpc::string output;
  {
    if (file->service_count() == 0) {
      return output;
    }
    auto out = file->CreatePrinter(&output);
    out->Print("<?php\n");
    out->Print("// GENERATED CODE -- DO NOT EDIT!\n\n");

    PrintServices(file, out.get());
  }
  return output;
}

}  // namespace grpc_php_generator
