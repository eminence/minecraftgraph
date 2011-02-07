#ifndef _NBT_H
#define _NBT_H

#include <string>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <list>
#include <assert.h>
#include <zlib.h>


using namespace std;

enum TAG_Type {
    TAG_End = 0,
    TAG_Byte,
    TAG_Short,
    TAG_Int,
    TAG_Long,
    TAG_Float,
    TAG_Double,
    TAG_Byte_Array,
    TAG_String,
    TAG_List,
    TAG_Compound
};



class NBT_Tag {
    public:
        TAG_Type type;
        string name;
        NBT_Tag() {};
        unsigned int size;
        virtual void print(int indent = 0) = 0;
};

/*
class NBT_Compound : public NBT_Tag {
    

};
*/

class NBT_Int : public NBT_Tag {
    public:
        NBT_Int(const char* data);
        NBT_Int(gzFile gz);
        int getValue() {return int_data; }
        operator int() {return int_data; } 
        void print(int indent);
    private:
        int int_data;
};


class NBT_Short : public NBT_Tag {
    public:
        NBT_Short(char* data);
        NBT_Short(gzFile gz);
        short getValue() {return int_data; }
        operator short() {return int_data; } 
        void print(int indent);
    private:
        short int_data;
};


class NBT_String: public NBT_Tag  {
    public:
        NBT_String(char *data);
        NBT_String(gzFile);
        ~NBT_String();
        string getString() { return *buff; }
        void print(int indent);
    private:
        string *buff;

};


class NBT_Byte_Array: public NBT_Tag  {
    public:
        NBT_Byte_Array(char *data);
        NBT_Byte_Array(gzFile);
        ~NBT_Byte_Array();
        int length() {return _length;};
        void print(int indent);
        char *buff;
    private:
        int _length;

};

class NBT_Long: public NBT_Tag  {
    public:
        NBT_Long(char *data);
        NBT_Long(gzFile);
        void print(int indent);
    private:
        uint64_t _data;

};

class NBT_Float: public NBT_Tag  {
    public:
        NBT_Float(char *data);
        NBT_Float(gzFile gz);
        void print(int indent);
    private:
        uint32_t _data;

};

class NBT_Double: public NBT_Tag  {
    public:
        NBT_Double(char *data);
        NBT_Double(gzFile gz);
        void print(int indent);
    private:
        uint64_t _data;

};

class NBT_Byte: public NBT_Tag  {
    public:
        NBT_Byte(char *data);
        NBT_Byte(gzFile);
        char getByte() { return _data; };
        void print(int indent);
    private:
        char _data;

};


class NBT_List: public NBT_Tag {
    public:
        NBT_List(char *data);
        NBT_List(gzFile);
        ~NBT_List();

        TAG_Type getListType() { return _type;};
        list<NBT_Tag*> getList() { return _list;};
        void print(int indent);
    private:
        TAG_Type _type;
        list<NBT_Tag*> _list;


};

class NBT_Compound: public NBT_Tag {
    public:
        NBT_Compound(char *data);
        NBT_Compound(gzFile gz);
        ~NBT_Compound();

        void print(int indent);
        list<NBT_Tag*> getList() { return _list; };
        NBT_Tag *findByType();
        NBT_Tag *findByName(string, int);
    private:
        list<NBT_Tag*> _list;
};

#endif // _NBT_H
