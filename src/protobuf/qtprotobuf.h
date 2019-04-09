/*
 * MIT License
 *
 * Copyright (c) 2019 Alexey Edelev <semlanik@gmail.com>
 *
 * This file is part of qtprotobuf project https://git.semlanik.org/semlanik/qtprotobuf
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

#pragma once

#include <qtprotobuftypes.h>
#include <protobufobject.h>
#include <qtprotobuflogging.h>
#include <QDebug>

//registerProtobufType Not a part of API
#define registerProtobufType(X) qRegisterMetaType<X>(# X);\
                                qRegisterMetaType<X>("qtprotobuf::"# X);

namespace qtprotobuf {

class QtProtobuf {
public:
    static void init() {
        static bool registationDone = false;
        if (!registationDone) {
            registerProtobufType(int32);
            registerProtobufType(int64);
            registerProtobufType(uint32);
            registerProtobufType(uint64);
            registerProtobufType(sint32);
            registerProtobufType(sint64);
            registerProtobufType(fint32);
            registerProtobufType(fint64);
            registerProtobufType(sfint32);
            registerProtobufType(sfint64);

            registerProtobufType(int32List);
            registerProtobufType(int64List);
            registerProtobufType(uint32List);
            registerProtobufType(uint64List);
            registerProtobufType(sint32List);
            registerProtobufType(sint64List);
            registerProtobufType(fint32List);
            registerProtobufType(fint64List);
            registerProtobufType(sfint32List);
            registerProtobufType(sfint64List);

            registerProtobufType(DoubleList);
            registerProtobufType(FloatList);
            ProtobufObjectPrivate::registerSerializers();
            registationDone = true;
        }
    }
};

}

#undef registerProtobufType
