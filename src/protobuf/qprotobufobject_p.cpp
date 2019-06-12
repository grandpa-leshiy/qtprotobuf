/*
 * MIT License
 *
 * Copyright (c) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
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

#include "qprotobufobject_p.h"

using namespace qtprotobuf;

ProtobufObjectPrivate::SerializerRegistry ProtobufObjectPrivate::serializers = {};

void ProtobufObjectPrivate::registerSerializers()
{
    wrapSerializer<float>(ProtobufObjectPrivate::serializeBasic<float>, ProtobufObjectPrivate::deserializeBasic<float>, Fixed32);
    wrapSerializer<double>(ProtobufObjectPrivate::serializeBasic<double>, ProtobufObjectPrivate::deserializeBasic<double>, Fixed64);
    wrapSerializer<int32>(ProtobufObjectPrivate::serializeBasic<int32>, ProtobufObjectPrivate::deserializeBasic<int32>, Varint);
    wrapSerializer<int64>(ProtobufObjectPrivate::serializeBasic<int64>, ProtobufObjectPrivate::deserializeBasic<int64>, Varint);
    wrapSerializer<uint32>(ProtobufObjectPrivate::serializeBasic<uint32>, ProtobufObjectPrivate::deserializeBasic<uint32>, Varint);
    wrapSerializer<uint64>(ProtobufObjectPrivate::serializeBasic<uint64>, ProtobufObjectPrivate::deserializeBasic<uint64>, Varint);
    wrapSerializer<sint32>(ProtobufObjectPrivate::serializeBasic<sint32>, ProtobufObjectPrivate::deserializeBasic<sint32>, Varint);
    wrapSerializer<sint64>(ProtobufObjectPrivate::serializeBasic<sint64>, ProtobufObjectPrivate::deserializeBasic<sint64>, Varint);
    wrapSerializer<fixed32>(ProtobufObjectPrivate::serializeBasic<fixed32>, ProtobufObjectPrivate::deserializeBasic<fixed32>, Fixed32);
    wrapSerializer<fixed64>(ProtobufObjectPrivate::serializeBasic<fixed64>, ProtobufObjectPrivate::deserializeBasic<fixed64>, Fixed64);
    wrapSerializer<sfixed32>(ProtobufObjectPrivate::serializeBasic<sfixed32>, ProtobufObjectPrivate::deserializeBasic<sfixed32>, Fixed32);
    wrapSerializer<sfixed64>(ProtobufObjectPrivate::serializeBasic<sfixed64>, ProtobufObjectPrivate::deserializeBasic<sfixed64>, Fixed64);
    wrapSerializer<bool>(ProtobufObjectPrivate::serializeBasic<uint32>, ProtobufObjectPrivate::deserializeBasic<uint32>, Varint);

    wrapSerializer<QString>([](const QString &data, int &/*fieldIndex*/) {
        return serializeLengthDelimited(data);
    }, [](SelfcheckIterator &it){
        return QVariant::fromValue(QString::fromUtf8(deserializeLengthDelimited(it)));
    }, LengthDelimited);

    wrapSerializer<QByteArray>([](const QByteArray &data, int &/*fieldIndex*/) {
        return serializeLengthDelimited(data);
    }, ProtobufObjectPrivate::deserializeLengthDelimited, LengthDelimited);

    wrapSerializer<FloatList>(ProtobufObjectPrivate::serializeListType<float>, ProtobufObjectPrivate::deserializeList<float>, LengthDelimited);
    wrapSerializer<DoubleList>(ProtobufObjectPrivate::serializeListType<double>, ProtobufObjectPrivate::deserializeList<double>, LengthDelimited);
    wrapSerializer<fixed32List>(ProtobufObjectPrivate::serializeListType<fixed32>, ProtobufObjectPrivate::deserializeList<fixed32>, LengthDelimited);
    wrapSerializer<fixed64List>(ProtobufObjectPrivate::serializeListType<fixed64>, ProtobufObjectPrivate::deserializeList<fixed64>, LengthDelimited);
    wrapSerializer<sfixed32List>(ProtobufObjectPrivate::serializeListType<sfixed32>, ProtobufObjectPrivate::deserializeList<sfixed32>, LengthDelimited);
    wrapSerializer<sfixed64List>(ProtobufObjectPrivate::serializeListType<sfixed64>, ProtobufObjectPrivate::deserializeList<sfixed64>, LengthDelimited);
    wrapSerializer<int32List>(ProtobufObjectPrivate::serializeListType<int32>, ProtobufObjectPrivate::deserializeList<int32>, LengthDelimited);
    wrapSerializer<int64List>(ProtobufObjectPrivate::serializeListType<int64>, ProtobufObjectPrivate::deserializeList<int64>, LengthDelimited);
    wrapSerializer<sint32List>(ProtobufObjectPrivate::serializeListType<sint32>, ProtobufObjectPrivate::deserializeList<sint32>, LengthDelimited);
    wrapSerializer<sint64List>(ProtobufObjectPrivate::serializeListType<sint64>, ProtobufObjectPrivate::deserializeList<sint64>, LengthDelimited);
    wrapSerializer<uint32List>(ProtobufObjectPrivate::serializeListType<uint32>, ProtobufObjectPrivate::deserializeList<uint32>, LengthDelimited);
    wrapSerializer<uint64List>(ProtobufObjectPrivate::serializeListType<uint64>, ProtobufObjectPrivate::deserializeList<uint64>, LengthDelimited);
    wrapSerializer<QStringList>(ProtobufObjectPrivate::serializeListType<QString>, ProtobufObjectPrivate::deserializeList<QString>, LengthDelimited);
    wrapSerializer<QByteArrayList>(ProtobufObjectPrivate::serializeListType<QByteArray>, ProtobufObjectPrivate::deserializeList<QByteArray>, LengthDelimited);
}

QByteArray ProtobufObjectPrivate::serializeProperty(const QVariant &propertyValue, int fieldIndex, const QMetaProperty &metaProperty)
{
    qProtoDebug() << __func__ << "propertyValue" << propertyValue << "fieldIndex" << fieldIndex
                  << "typeName" << metaProperty.typeName() << static_cast<QMetaType::Type>(propertyValue.type());

    QByteArray result;
    WireTypes type = UnknownWireType;

    if (metaProperty.isEnumType()) {
        type = Varint;
        result.append(serializeBasic(int64(propertyValue.value<int32_t>()), fieldIndex));
    } else {
        result.append(serializeUserType(propertyValue, fieldIndex, type));
    }

    if (fieldIndex != NotUsedFieldIndex
            && type != UnknownWireType) {
        result.prepend(encodeHeader(fieldIndex, type));
    }
    return result;
}

QByteArray ProtobufObjectPrivate::serializeUserType(const QVariant &propertyValue, int &fieldIndex, WireTypes &type)
{
    qProtoDebug() << __func__ << "propertyValue" << propertyValue << "fieldIndex" << fieldIndex;

    int userType = propertyValue.userType();
    auto serializer = serializers.at(userType);//Throws exception if not found
    type = serializer.type;
    return serializer.serializer(propertyValue, fieldIndex);
}

void ProtobufObjectPrivate::deserializeProperty(QObject *object, const QMetaProperty &metaProperty, SelfcheckIterator &it, WireTypes wireType)
{
    qProtoDebug() << __func__ << " wireType: " << wireType << " metaProperty: " << metaProperty.typeName()
                  << "currentByte:" << QString::number((*it), 16);

    QVariant newPropertyValue;
    if (metaProperty.isEnumType()) {
        newPropertyValue = QVariant::fromValue(int32_t(deserializeBasic<int64>(it).value<int64>()._t));
    } else {
        newPropertyValue = metaProperty.read(object);
        deserializeUserType(metaProperty, it, newPropertyValue);
    }
    metaProperty.write(object, newPropertyValue);
}

void ProtobufObjectPrivate::deserializeUserType(const QMetaProperty &metaProperty, SelfcheckIterator &it, QVariant &out)
{
    qProtoDebug() << __func__ << "userType" << metaProperty.userType() << "typeName" << metaProperty.typeName()
                  << "currentByte:" << QString::number((*it), 16);
    int userType = metaProperty.userType();
    auto deserializer = serializers.at(userType).deserializer;//Throws exception if not found
    deserializer(it, out);
}

void ProtobufObjectPrivate::skipVarint(SelfcheckIterator &it)
{
    while ((*it) & 0x80) {
        ++it;
    }
    ++it;
}

void ProtobufObjectPrivate::skipLengthDelimited(SelfcheckIterator &it)
{
    //Get length of lenght-delimited field
    unsigned int length = deserializeBasic<uint32>(it).toUInt();
    it += length;
}

int ProtobufObjectPrivate::skipSerializedFieldBytes(SelfcheckIterator &it, WireTypes type)
{
    const auto initialIt = QByteArray::const_iterator(it);
    switch (type) {
    case WireTypes::Varint:
        skipVarint(it);
        break;
    case WireTypes::Fixed32:
        it += sizeof(decltype(fixed32::_t));
        break;
    case WireTypes::Fixed64:
        it += sizeof(decltype(fixed64::_t));
        break;
    case WireTypes::LengthDelimited:
        skipLengthDelimited(it);
        break;
    case WireTypes::UnknownWireType:
    default:
        throw std::invalid_argument("Cannot skip due to undefined length of the redundant field.");
    }

    return std::distance(initialIt, QByteArray::const_iterator(it));
}
