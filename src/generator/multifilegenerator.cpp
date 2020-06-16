/*
 * MIT License
 *
 * Copyright (c) 2019 Alexey Edelev <semlanik@gmail.com>
 *
 * This file is part of QtProtobuf project https://git.semlanik.org/semlanik/qtprotobuf
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "multifilegenerator.h"
#include "templates.h"

#include "messagedeclarationprinter.h"
#include "messagedefinitionprinter.h"
#include "enumdeclarationprinter.h"
#include "enumdefinitionprinter.h"

#include "serverdeclarationprinter.h"
#include "clientdeclarationprinter.h"
#include "clientdefinitionprinter.h"
#include "utils.h"
#include "generatoroptions.h"

#include <iostream>
#include <set>
#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.h>

using namespace ::QtProtobuf::generator;
using namespace ::google::protobuf;
using namespace ::google::protobuf::compiler;

MultiFileGenerator::MultiFileGenerator() : GeneratorBase(GeneratorBase::MultiMode)
{}

bool MultiFileGenerator::Generate(const FileDescriptor *file,
                           const std::string &,
                           GeneratorContext *generatorContext,
                           std::string *error) const
{
    if (file->syntax() != FileDescriptor::SYNTAX_PROTO3) {
        *error = "Invalid proto used. This plugin only supports 'proto3' syntax";
        return false;
    }

    for (int i = 0; i < file->message_type_count(); i++) {
        const Descriptor *message = file->message_type(i);

        //Detect nested fields and filter maps fields
        int mapsFieldsCount = 0;
        for (int j = 0; j < message->nested_type_count(); j++) {
            for (int k = 0; k < message->field_count(); k++) {
                if (message->field(k)->is_map() && message->field(k)->message_type() == message->nested_type(j)) {
                    ++mapsFieldsCount;
                }
            }
        }

        if (message->nested_type_count() > 0 && message->nested_type_count() > mapsFieldsCount) {
            std::cerr << file->name() << ":" << (message->index() + 1) << ": " << " Error: Meta object features not supported for nested classes in " << message->full_name() << std::endl;
            continue;
        }

        std::string baseFilename(message->name());
        utils::tolower(baseFilename);
        baseFilename = generateBaseName(file, baseFilename);

        std::unique_ptr<io::ZeroCopyOutputStream> headerStream(generatorContext->Open(baseFilename + ".h"));
        std::unique_ptr<io::ZeroCopyOutputStream> sourceStream(generatorContext->Open(baseFilename + ".cpp"));

        std::shared_ptr<::google::protobuf::io::Printer> headerPrinter(new ::google::protobuf::io::Printer(headerStream.get(), '$'));
        std::shared_ptr<::google::protobuf::io::Printer> sourcePrinter(new ::google::protobuf::io::Printer(sourceStream.get(), '$'));

        MessageDeclarationPrinter messageDecl(message, headerPrinter);

        printDisclaimer(headerPrinter);
        printPreamble(headerPrinter);

        headerPrinter->Print(Templates::DefaultProtobufIncludesTemplate);
        if (GeneratorOptions::instance().hasQml()) {
            headerPrinter->Print(Templates::QmlProtobufIncludesTemplate);
        }

        std::set<std::string> existingIncludes;
        for (int i = 0; i < message->field_count(); i++) {
            printInclude(headerPrinter, message, message->field(i), existingIncludes);
        }

        for (int i = 0; i < message->field_count(); i++) {
            auto field = message->field(i);
            if (field->type() == FieldDescriptor::TYPE_MESSAGE
                    && !field->is_map() && !field->is_repeated()) {
                auto dependency = field->message_type();
                auto namespaces = common::getNamespaces(dependency);
                printNamespaces(headerPrinter, namespaces);
                headerPrinter->Print({{"classname", utils::upperCaseName(dependency->name())}}, Templates::ProtoClassDeclarationTemplate);
                headerPrinter->Print("\n");
                for (size_t i = 0; i < namespaces.size(); ++i) {
                    headerPrinter->Print(Templates::SimpleBlockEnclosureTemplate);
                }
                headerPrinter->Print("\n");
            }
        }

        messageDecl.run();

        printDisclaimer(sourcePrinter);
        std::string includeFileName = message->name();
        utils::tolower(includeFileName);
        sourcePrinter->Print({{"include", includeFileName}}, Templates::InternalIncludeTemplate);
        if (GeneratorOptions::instance().hasQml()) {
            sourcePrinter->Print({{"include", "QQmlEngine"}}, Templates::ExternalIncludeTemplate);
        }

        MessageDefinitionPrinter messageDef(message, sourcePrinter);
        messageDef.run();
    }

    for (int i = 0; i < file->service_count(); i++) {
        const ServiceDescriptor *service = file->service(i);
        std::string baseFilename(service->name());
        utils::tolower(baseFilename);
        baseFilename = generateBaseName(file, baseFilename);

        std::unique_ptr<io::ZeroCopyOutputStream> headerStream(generatorContext->Open(baseFilename + "client.h"));
        std::unique_ptr<io::ZeroCopyOutputStream> sourceStream(generatorContext->Open(baseFilename + "client.cpp"));
        std::shared_ptr<::google::protobuf::io::Printer> headerPrinter(new ::google::protobuf::io::Printer(headerStream.get(), '$'));
        std::shared_ptr<::google::protobuf::io::Printer> sourcePrinter(new ::google::protobuf::io::Printer(sourceStream.get(), '$'));

        printDisclaimer(headerPrinter);
        printPreamble(headerPrinter);
        ClientDeclarationPrinter clientGen(service, headerPrinter);
        clientGen.printIncludes();
        clientGen.printClientIncludes();
        clientGen.run();

        printDisclaimer(sourcePrinter);
        std::string includeFileName = service->name();
        utils::tolower(includeFileName);
        sourcePrinter->Print({{"include", includeFileName}}, Templates::InternalIncludeTemplate);
        if (GeneratorOptions::instance().hasQml()) {
            sourcePrinter->Print({{"include", "QQmlEngine"}}, Templates::ExternalIncludeTemplate);
        }

        printQtProtobufUsingNamespace(sourcePrinter);
        ClientDefinitionPrinter clientSrcGen(service, sourcePrinter);
        clientSrcGen.run();
    }
    return true;
}

bool MultiFileGenerator::GenerateAll(const std::vector<const FileDescriptor *> &files, const string &parameter, GeneratorContext *generatorContext, string *error) const
{
    std::string globalEnumsFilename = "globalenums";

    std::unique_ptr<io::ZeroCopyOutputStream> headerStream(generatorContext->Open(globalEnumsFilename + ".h"));
    std::unique_ptr<io::ZeroCopyOutputStream> sourceStream(generatorContext->Open(globalEnumsFilename + ".cpp"));
    std::shared_ptr<::google::protobuf::io::Printer> headerPrinter(new ::google::protobuf::io::Printer(headerStream.get(), '$'));
    std::shared_ptr<::google::protobuf::io::Printer> sourcePrinter(new ::google::protobuf::io::Printer(sourceStream.get(), '$'));

    printDisclaimer(headerPrinter);
    printPreamble(headerPrinter);
    headerPrinter->Print(Templates::DefaultProtobufIncludesTemplate);

    printDisclaimer(sourcePrinter);
    sourcePrinter->Print({{"include", globalEnumsFilename}}, Templates::InternalIncludeTemplate);
    if (GeneratorOptions::instance().hasQml()) {
        sourcePrinter->Print({{"include", "QQmlEngine"}}, Templates::ExternalIncludeTemplate);
    }

    for (auto file : files) { //TODO: Each should be printed to separate file
        for (int i = 0; i < file->enum_type_count(); i++) {
            auto enumType = file->enum_type(i);
            EnumDeclarationPrinter enumDecl(enumType, headerPrinter);
            enumDecl.run();

            EnumDefinitionPrinter enumDef(enumType, sourcePrinter);
            enumDef.run();
        }
    }

    return GeneratorBase::GenerateAll(files, parameter, generatorContext, error);
}

