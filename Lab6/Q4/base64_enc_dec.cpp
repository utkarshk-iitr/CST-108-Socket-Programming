#include <bits/stdc++.h>
using namespace std;

const string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string encode(vector<unsigned char>& data){
    string res;
    int val=0,valb=-6;

    for(auto c:data){
        val = (val<<8)+c;
        valb += 8;
        while(valb>=0){
            res.push_back(b64[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }

    if(valb>-6){
        res.push_back(b64[((val<<8)>>(valb+8))&0x3F]);
    }

    while(res.size()%4){    
        res.push_back('=');
    }
    
    string formatted_res;
    for(int i=0;i<res.size();i++) {
        formatted_res.push_back(res[i]);
        if((i+1)%64==0 && i!=res.size()-1){
            formatted_res.push_back('\n');
        }
    }
    
    return formatted_res;
}

vector<unsigned char> decode(string &in){
    vector<int> T(256,-1);
    vector<unsigned char> out;
    int val=0,valb=-8;
    
    for(int i=0;i<64;i++){
        T[(unsigned char)b64[i]] = i;
    }

    for(auto c:in){
        if(T[c]==-1) 
            break;
        val = (val<<6)+T[c];
        valb += 6;
        if(valb>=0){
            out.push_back((unsigned char)((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return out;
}

int main(int argc,char** argv){
    if(argc<2){
        cerr<<"Usage: "<<argv[0]<<" -e|-d\n";
        return 1;
    }
    string mode = argv[1];
    vector<unsigned char> input;
    
    char buf[4096];

    while(cin.read(buf,sizeof(buf)))
        input.insert(input.end(),buf,buf+cin.gcount());
        
    if(cin.gcount()) 
        input.insert(input.end(),buf,buf+cin.gcount());

    if (mode=="-e"){
        cout<<encode(input)<<endl;
        return 0;
    } 
    
    else if(mode=="-d"){
        string s(input.begin(),input.end());
        string cleaned;

        for(auto c: s){
            if(!isspace((unsigned char)c)) 
                cleaned.push_back(c);
        }

        size_t eq = cleaned.find('=');
        if (eq!=string::npos)
            cleaned = cleaned.substr(0,eq);

        vector<unsigned char> out = decode(cleaned);
        for(auto c:out) cout<<c;
    } 
    
    else {
        cerr<<"Unknown option\n";
        return 1;
    }
    return 0;
}
