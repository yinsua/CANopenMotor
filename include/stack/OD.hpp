#ifndef CANOPEN_OD_H
#define CANOPEN_OD_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <variant>

class OD{
private:
    using OD_Container = std::map<std::string, std::map<std::string, std::string>>;
    std::string path_;
    std::ifstream od_file_stream_;
    OD_Container od_;

    bool is_title(const std::string& s){
        if(s.front() == '[' ){
            return true;
        }
        return false;
    }

    bool is_index(const std::string& s){
        uint8_t i = s.at(1);
        if(i >= 48/*0*/ && i <= 57/*9*/){
            return true;
        }
        return false;
    }

public:
    using OD_Ptr = std::shared_ptr<OD>;
    OD(std::string path)
        : path_(path)
    {
    }

    bool init(){
        od_file_stream_.open(path_);
        if(od_file_stream_.is_open()){
            std::string line;
            std::string current_index="";
            while(getline(od_file_stream_, line)){
                if(is_title(line)){
                    current_index = line.substr(1, line.find(']')-1);
                }
                else{
                    if(current_index == "") continue;
                    
                    std::string::size_type n = line.find('=');
                    if(n != std::string::npos){
                        std::string key, value;
                        key = line.substr(0, n);
                        value = line.substr(n+1);
                        // od_[current_index][key] = value;
                        // std::cout<<key<<":"<<value<<std::endl;
                        auto &index = od_[current_index];
                        index[key] = value;
                    }
                }
            }
#if 0
            for(auto e1 : od_){
                std::cout<<e1.first<<std::endl;
                for(auto e2 : e1.second){
                    // std::cout<<"\t"<<e2.first<<":"<<e2.second<<std::endl;
                    if(e2.first == "DataType")
                        std::cout<<"\t"<<std::stoi(e2.second, 0, 16)<<std::endl;
                }
            }
#endif            
        }
        return false;
    }

    typedef enum : int{
        INT8_Type = 2,
        INT16_Type = 3,
        INT32_Type = 4,
        UINT8_Type = 5,
        UINT16_Type = 6,
        UINT32_Type = 7
    }OD_DataType;

    uint8_t get_type(int index, int sub_index=-1){
        auto int2str = [](int data)->std::string{
            char c[1024];
            sprintf(c, "%X", data);
            std::string str(c);
            return str;
        };

        if(sub_index == -1){
            auto title = od_[int2str(index)];
            return std::stoi(title["DataType"], 0, 16);
        }
        else{
            return std::stoi(od_[int2str(index)+"sub"+int2str(sub_index)]["DataType"], 0, 16);
        }
    }

    uint8_t get_byte(int index, int sub_index=-1)
    {
        auto type = get_type(index, sub_index);
        uint8_t result = 0;
        switch(type)
        {
            case 2:
            case 5: result = 1;break;
            case 3:
            case 6: result = 2;break;
            case 4:
            case 7: result = 4;break;
            default: std::cerr<<"get byte error, invalid type"<<std::endl;break;
        }
        return result;
    }

    /**
     *  return value: 
     *      true -> signed      false -> unsigned
     */ 
    bool get_signed(int index, int sub_index=-1)
    {
        auto type = get_type(index, sub_index);
        bool result = false;
        switch(type)
        {
            case 2:
            case 3: 
            case 4: result = true;break;
            case 5: 
            case 6:
            case 7: result = false;break;
            default: std::cerr<<"get signed error, invalid type"<<std::endl;break;
        }
        return result;
    }
};

#endif