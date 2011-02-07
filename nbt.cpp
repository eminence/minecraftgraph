#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <zlib.h>

#include "nbt.h"


void gzAssert(gzFile gz, int actual, int expected) {
    if (actual != expected) {
        printf("actual(%d) != expected(%d)\n", actual, expected);
        int e;
        printf("gzerror: %s\n", gzerror(gz, &e));
        assert(0 && "Run in a debugger to get a backtrace");
    }

}

////////////////////////////////////////////////////////////////////////////////
// NBT_Int
////////////////////////////////////////////////////////////////////////////////
NBT_Int::NBT_Int(gzFile gz) {
    type = TAG_Int;

    // grab 4 bytes
    int buf;
    assert(gzread(gz, &buf, 4) == 4);
    int_data = ntohl(buf);

    size = 4;

}
NBT_Int::NBT_Int(const char* data) {
    type = TAG_Int;

    // grab 4 bytes
    int buf;
    memcpy(&buf, data, 4);
    int_data = ntohl(buf);

    size = 4;

}


////////////////////////////////////////////////////////////////////////////////
// NBT_List
////////////////////////////////////////////////////////////////////////////////
void NBT_Int::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Int(%s) : %d\n", name.c_str(), int_data);
    free(spacing);
};



NBT_List::~NBT_List() {
    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        //printf("NBT_List: deleting\n");
        delete *iter;
    }
}

NBT_List::NBT_List(gzFile gz) {
    type = TAG_List;

    // list types
    NBT_Byte t_byte(gz);

    _type = (TAG_Type)t_byte.getByte();

    // list lenght
    NBT_Int t_int(gz);
    //printf("len %d\n", t_int.getValue());

    for (int i = 0; i < t_int.getValue(); i++) {
        NBT_Tag *ele;
        switch (_type) {
            case TAG_Int:
                ele = new NBT_Int(gz);
                break;
            case TAG_Short:
                ele = new NBT_Short(gz);
                break;
            case TAG_Byte:
                ele = new NBT_Byte(gz);
                break;
            case TAG_String:
                ele = new NBT_String(gz);
                break;
            case TAG_Long:
                ele = new NBT_Long(gz);
                break;
            case TAG_Double:
                ele = new NBT_Double(gz);
                break;
            case TAG_Float:
                ele = new NBT_Float(gz);
                break;
            case TAG_Byte_Array:
                ele = new NBT_Byte_Array(gz);
                break;
            case TAG_List:
                ele = new NBT_List(gz);
                break;
            case TAG_Compound:
                ele = new NBT_Compound(gz);
                break;
            default:
                printf("type: %d\n", _type);
                assert(0 && "Unknown type!");

        }
        _list.push_front(ele);
    }


}

NBT_List::NBT_List(char *data) {
    char *start = data;
    type = TAG_List;

    // list types
    NBT_Byte t_byte(data);
    data += t_byte.size;

    _type = (TAG_Type)t_byte.getByte();
    //printf("type %d\n", _type);

    // list lenght
    NBT_Int t_int(data);
    data += t_int.size;
    //printf("len %d\n", t_int.getValue());

    for (int i = 0; i < t_int.getValue(); i++) {
        NBT_Tag *ele;
        switch (_type) {
            case TAG_Int:
                ele = new NBT_Int(data);
                break;
            case TAG_Short:
                ele = new NBT_Short(data);
                break;
            case TAG_Byte:
                ele = new NBT_Byte(data);
                break;
            case TAG_String:
                ele = new NBT_String(data);
                break;
            case TAG_Long:
                ele = new NBT_Long(data);
                break;
            case TAG_Double:
                ele = new NBT_Double(data);
                break;
            case TAG_Float:
                ele = new NBT_Float(data);
                break;
            case TAG_Byte_Array:
                ele = new NBT_Byte_Array(data);
                break;
            case TAG_List:
                ele = new NBT_List(data);
                break;
            case TAG_Compound:
                ele = new NBT_Compound(data);
                break;
            default:
                printf("type: %d\n", _type);
                assert(0 && "Unknown type!");

        }
        data+= ele->size;
        _list.push_front(ele);
    }

    size= (data - start);
}


void
NBT_List::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);
    printf("TAG_List(\"%s\") %d entries of type %d \n", name.c_str(), _list.size(), _type);
    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        NBT_Tag *item = *iter;
        item->print(indent + 4);

    }
    free(spacing);
};

////////////////////////////////////////////////////////////////////////////////
// NBT_Double
////////////////////////////////////////////////////////////////////////////////
NBT_Double::NBT_Double(gzFile gz) {
    type = TAG_Double;
    assert(gzread(gz, &_data, 8) == 8);
}

NBT_Double::NBT_Double(char *data) {
    type = TAG_Double;
    memcpy(&_data, data, 8);
    size=8;

}


void NBT_Double::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    memset(spacing, 0, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Double(%s) \n", name.c_str());
    free(spacing);
};


////////////////////////////////////////////////////////////////////////////////
// NBT_Compound
////////////////////////////////////////////////////////////////////////////////
NBT_Compound::NBT_Compound(gzFile gz) {
    type = TAG_Compound;

    char next_type;
    assert(gzread(gz, &next_type, 1) == 1);
    while (next_type != TAG_End) {
        //printf("next type: %d\n", next_type);

        NBT_String name(gz);

        //printf("next name: %s\n", name.getString().c_str());

        NBT_Tag *ele;
        switch (next_type) {
            case TAG_Int:
                ele = new NBT_Int(gz);
                break;
            case TAG_Short:
                ele = new NBT_Short(gz);
                break;
            case TAG_Long:
                ele = new NBT_Long(gz);
                break;
            case TAG_Byte:
                ele = new NBT_Byte(gz);
                break;
            case TAG_String:
                ele = new NBT_String(gz);
                break;
            case TAG_Byte_Array:
                ele = new NBT_Byte_Array(gz);
                break;
            case TAG_Double:
                ele = new NBT_Double(gz);
                break;
            case TAG_Float:
                ele = new NBT_Float(gz);
                break;
            case TAG_Compound:
                ele = new NBT_Compound(gz);
                break;
            case TAG_List:
                ele = new NBT_List(gz);
                break;

            default:
                printf("type: %d\n", next_type);
                assert(0 && "Unknown type!");

        }

        ele->name = name.getString();
        _list.push_front(ele);


        //gzAssert(gz, gzread(gz, &next_type, 1), 1);
        if (gzread(gz, &next_type, 1) != 1)
            return;


    }
    //size = (data - start);


}

NBT_Compound::NBT_Compound(char *data) {
    type = TAG_Compound;
    char *start = data;

    char next_type = *data;
    data++;
    while (next_type != TAG_End) {
        //printf("next type: %d\n", next_type);

        NBT_String name(data);
        data += name.size;

        //printf("next name: %s\n", name.getString().c_str());

        NBT_Tag *ele;
        switch (next_type) {
            case TAG_Int:
                ele = new NBT_Int(data);
                break;
            case TAG_Short:
                ele = new NBT_Short(data);
                break;
            case TAG_Long:
                ele = new NBT_Long(data);
                break;
            case TAG_Byte:
                ele = new NBT_Byte(data);
                break;
            case TAG_String:
                ele = new NBT_String(data);
                break;
            case TAG_Byte_Array:
                ele = new NBT_Byte_Array(data);
                break;
            case TAG_Double:
                ele = new NBT_Double(data);
                break;
            case TAG_Float:
                ele = new NBT_Float(data);
                break;
            case TAG_Compound:
                ele = new NBT_Compound(data);
                break;
            case TAG_List:
                ele = new NBT_List(data);
                break;

            default:
                printf("type: %d\n", next_type);
                assert(0 && "Unknown type!");

        }
        data+= ele->size;
        ele->name = name.getString();
        _list.push_front(ele);


        next_type = *data;
        data++;


    }
    size = (data - start);
}

NBT_Compound::~NBT_Compound() {
    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        //printf("NBT_Compound: deleting\n");
        delete *iter;
    }
}

NBT_Tag * NBT_Compound::findByType() { return NULL; }
NBT_Tag * NBT_Compound::findByName(string n, int recursive) {
    
    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        NBT_Tag *item = *iter;
        if (item->name == n) return item;
    }
    if (!recursive) return NULL;
    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        NBT_Tag *item = *iter;
        if (item->type == TAG_Compound) {
            NBT_Tag *found = ((NBT_Compound*)item)->findByName(n, recursive);
            if (found) return found;
        }
    }

    return NULL;

}

void NBT_Compound::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%sTAG_Compound(\"%s\"): %d entries {\n", spacing, name.c_str(), _list.size());

    for(list<NBT_Tag*>::iterator iter = _list.begin(); iter != _list.end(); iter++) {
        NBT_Tag *item = *iter;
        item->print(indent + 4);

    }
    printf("%s}\n", spacing);
    free(spacing);

};



NBT_String::NBT_String(gzFile gz) {
    type = TAG_String;
    NBT_Short len(gz);

    int l = len.getValue();
    //printf("NBT_String; len: %d\n", l);

    char *buf = (char *)malloc(l);
    int r =  gzread(gz, buf, l);
    assert(r == l);

    buff = new string(buf, l);

    size = l + len.size;

}


NBT_String::NBT_String(char *data) {
    type = TAG_String;

    NBT_Short len(data);
    data += len.size;
    int l = len.getValue();
    //printf("NBT_Sring; len: %d\n",l);

    buff = new string(data, l);
    size = l + len.size;

}

NBT_String::~NBT_String() {
    //printf("deleting buff\n");
    delete buff;
}


void NBT_String::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%sTAG_String(%s) : %s\n", spacing, name.c_str(), buff->c_str());

    free(spacing);
};


////////////////////////////////////////////////////////////////////////////////
//  NBT_Float
////////////////////////////////////////////////////////////////////////////////

NBT_Float::NBT_Float(gzFile gz) {
    type = TAG_Float;
    assert(gzread(gz, &_data, 4) == 4);
    size=4;
}

NBT_Float::NBT_Float(char *data) {
    type = TAG_Float;
    memcpy(&_data, data, 4);
    size=4;

}


void NBT_Float::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Float(%s) \n", name.c_str());
    free(spacing);
};



////////////////////////////////////////////////////////////////////////////////
//  NBT_Short
////////////////////////////////////////////////////////////////////////////////
NBT_Short::NBT_Short(gzFile gz) {
    type = TAG_Short;
    int buf;
    assert(gzread(gz, &buf, 2) == 2);
    int_data = ntohs(buf);
}

NBT_Short::NBT_Short(char* data) {
    type = TAG_Short;
    // grab 4 bytes
    int buf;
    memcpy(&buf, data, 2);
    int_data = ntohs(buf);

    size=2;
}

void NBT_Short::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Short(%s) : %d\n", name.c_str(), int_data);
    free(spacing);
};


////////////////////////////////////////////////////////////////////////////////
//  NBT_Byte_Array
////////////////////////////////////////////////////////////////////////////////
NBT_Byte_Array::NBT_Byte_Array(gzFile gz) {
    type = TAG_Byte_Array;

    NBT_Int len(gz);
    int l = len.getValue();

    buff = (char*)malloc(l);
    assert(buff != NULL);

    _length = l;
    assert(gzread(gz, buff, l) == l);


}

NBT_Byte_Array::NBT_Byte_Array(char *data) {
    type = TAG_Byte_Array;

    NBT_Int len(data);
    data += len.size;
    int l = len.getValue();

    buff = (char*)malloc(l);
    assert(buff != NULL);
    _length = l;
    //printf("[debug] malloc %p size:%d\n", buff, l);
    memcpy(buff, data, l);
    size = len.size + l;
}


NBT_Byte_Array::~NBT_Byte_Array() {
    free((void*)buff);
    //printf("[debug] free %p\n", buff);
}


void NBT_Byte_Array::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Byte_Array(%s) length %d\n", name.c_str(), _length);
    free(spacing);
}

////////////////////////////////////////////////////////////////////////////////
// NBT_Byte
////////////////////////////////////////////////////////////////////////////////
NBT_Byte::NBT_Byte(gzFile gz) {
    type = TAG_Byte;
    assert(gzread(gz, &_data, 1) == 1);

}
NBT_Byte::NBT_Byte(char *data) {
    type = TAG_Byte;

    memcpy(&_data, data, 1);
    size=1;
}

void NBT_Byte::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);
    printf("TAG_Byte(%s) \n", name.c_str());
    free(spacing);
}


////////////////////////////////////////////////////////////////////////////////
// NBT_Long
////////////////////////////////////////////////////////////////////////////////
NBT_Long::NBT_Long(gzFile gz) {
    type = TAG_Long;
    assert(gzread(gz, &_data, 8) == 8);


}

NBT_Long::NBT_Long(char *data) {
    type = TAG_Long;
    memcpy(&_data, data, 8);
    size=8;

}

void NBT_Long::print(int indent) {
    char *spacing = (char*)malloc(indent+1);
    bzero(spacing, indent+1);
    memset(spacing, ' ', indent);
    printf("%s", spacing);

    printf("TAG_Long(%s) \n", name.c_str());
    free(spacing);
}

