/* 
NAME : ANIRUDH D BHAT
ROLL NUMBER: 2201CS94
DECLARATION OF AUTHORSHIP: I HEREBY DECLARE THAT THE SOURCE CODE I AM SUBMITTING IS ENTIRELY MY OWN ORIGINAL WORK EXCEPT WHERE OTHERWISE INDICATED.
*/

#include <bits/stdc++.h>
using namespace std;

//all variables and data structures
string fileName;//storing the file name in order to make the 
vector<vector<string>>source_code(10000);//Stores each non empty line
vector<string>machine_code(10000);//Holds the machine code in hex
int machine_code_obj[10000];//Used to create the object code
vector<pair<int,string>>errors;//Stores all the errors encounteres
vector<pair<int,string>>warnings;//Stores all the warnings encountered
bool halt_flag=false;//Used to check if halt is implemented or not
map<string,char>bin_to_hex;//stores conversion of binary to hex
map<string,int>inst_0_op;//stores the mnemonics and opcode of 0operand instructions
map<string,int>inst_1_op;//stores the mnemonics and opcode of 1 operand instructions
map<string,int>inst_1_of;//stores the mnemonics and opcode of 1 offset instructions
map<string,int>labels;//stores the label names and corresponding address
map<string,int>labels_line;
set<string>used_labels;

// ALL FUNCTIONS USED
void init();//initializes the mnemonics and opcodes and conversions
void identify_warnings();
void print_warnings();
void print_errors();
void generate_log();
void generate_list();
void generate_obj();
void SET();
bool isValidLabel(string s);
int isValid(string s);//Checks if the operand is a valid label or number
string decl_to_hexa(int num,int no_bits);//converts a decimal to corresponding hexadecimal(32bit)
int convert_to_decimal(string s,int line);//converts string to decimal
string clean(string s,int len);//trims spaces
void first_pass(string s,int line_num,int* pc);//inital pass to check errors and store labels and each line
void second_pass();//makes the machine code and also checks for remaining errors
void make_machine_code(int instruction_type,string mnemonic,string operand,int pc,string label_to_be_used);


int main(int argc,char* argv[])
{
    if(argc!=2)//check if valid input
    {
        cout<<"Invalid file input, It must be ./asm filename.asm"<<endl;
        exit(0);
    }
    init();//initialize fn
    fileName="";
    //file name extraction so as to append to the outputfiles
    for(int i = 0; i<strlen(argv[1]);i++)
    {
        if(argv[1][i]=='.')
        break;
        fileName= fileName+argv[1][i];
    }
    fstream if_ptr;
    if_ptr.open(argv[1],ios::in);

    //check if it opens, then proceeds to extract each line and go through first pass to identify labels and errors
    if(if_ptr.is_open())
    {
        int line = 1;
        string current;
        int pc = 0;

        while(getline(if_ptr,current))
        {
            string k = "";
            for(int i = 0; i<current.length();i++)
            {
                if(current[i]==';')
                break;
                k=k+current[i];
            }
            //cout<<k<<endl;
            if(k!="")//not empty then go to first pass, else just increase line number and move forward, pc stays the same, empty line not counted
            first_pass(k,line,&pc);

            line++;

        }
        SET();
		second_pass();// Used to convert given assmebly to machine code and along with that identify more errors 
		identify_warnings();
    }

    else
	{
		errors.push_back({0,"Unable to open given file, please check format of the file"});// Invalid file argument 
    }	
    
	generate_log();// Write lables info, errors and warnings to a log file
	generate_list();// generate listing file
	
	if(errors.empty())// Check if given assembly has errors or not 
	{		
		
		if(!warnings.empty())// Print warnings, if present
			print_warnings();
	
		generate_obj(); // generate object file		
	}
	else
	{
		
		print_errors();
		if(!warnings.empty())
			print_warnings();// print all errors along with existing warnings
	}
	if_ptr.close();// Close source file 
	return 0;
}

void init()
{
    inst_0_op={{"add",6},
                {"sub",7},
                {"shl",8},
                {"shr",9},
                {"a2sp",11},
                {"sp2a",12},
                {"return",14},
                {"HALT",18}
               };
    inst_1_op={
                {"ldc",0},
                {"adc",1},
                {"ldl",2},
                {"stl",3},
                {"ldnl",4},
                {"stnl",5},
                {"adj",10},
                {"call",13},
                {"brz",15},
                {"brlz",16},
                {"br",17},
                {"SET",19},
                {"data",20}
              } ;
    inst_1_of={
                {"call",13},
                {"brz",15},
                {"brlz",16},
                {"br",17}
              };
    bin_to_hex={
                {"0000",'0'},
                {"0001",'1'},
                {"0010",'2'},
                {"0011",'3'},
                {"0100",'4'},
                {"0101",'5'},
                {"0110",'6'},
                {"0111",'7'},
                {"1000",'8'},
                {"1001",'9'},
                {"1010",'A'},
                {"1011",'B'},
                {"1100",'C'},
                {"1101",'D'},
                {"1110",'E'},
                {"1111",'F'}
               } ;                
}

bool isValidLabel(string s)
{
    if(!(isalpha(s[0]) || s[0]=='_'))//check if the first character is alphabet or _
    return false;

    for(int i = 1; i<s.length();i++)
    {
        if(!(isalnum(s[i]) || s[i]=='_')) // check if all other characters are alphanumerical or _
        return false;
    }

    return true;
}

int isValid(string s) // 0-invalid number, 1-invalid label, 2-valid label,3-decimal,4-octal,5-hexadecimal,6:-is a instruction
{
    if(inst_0_op.find(s)!= inst_0_op.end() || inst_1_op.find(s)!= inst_1_op.end())
    {
        return 6;
    }
    if(isValidLabel(s))
    {
        if(labels.find(s) != labels.end())
        return 2;
        if(labels.find(s) == labels.end())
        return 1;
    }
    int len = s.length();
    if(s[0]=='0' && len>=2)
    {
        if(s[1]=='X' || s[1]=='x')
        {
            for(int i = 2 ; i<len;i++)
            {
                if(!(isdigit(s[i]) || ((s[i]<='f' && s[i]>='a') || (s[i]>='A' && s[i]<='F'))))
                return 0;
            }
            return 5;
        }
        else
        {
            for(int i = 1 ; i<len;i++)
            {
                if(!(s[i]>='0' && s[i]<='7'))
                return 0;
            }
            return 4;
        }
    }
    else
    {
        for(int i = 0 ; i<len;i++)
            {
                if(!(isdigit(s[i])))
                return 0;
            }
            return 3;
    }
}

string clean(string s,int len)
{
    reverse(s.begin(),s.end());//reverse string once
    while(s.back()==' ' || s.back()=='\t')//if there are leading zeroes pop it
    s.pop_back();

    reverse(s.begin(),s.end());//reverse it again
    while(s.back()==' ' || s.back()=='\t')//if there are trailing zeroes remove it
    s.pop_back();

    return s;
}

void first_pass(string s,int line,int *pc)
{
    int len = s.length();
    string k = clean(s,len);//get the string s without leading or trailing zeroes
    len = k.length();//find its length

    //cout<<k<<endl;
    if(k.empty())
    return;

    bool label = false;

    for(int i = 0; i<len; i++)//replace semicolon of the label with a space that can be used as a delimiter to then sperate it into respective words
    {
        if(k[i]==':')
        {
            k[i]=' ';
            label = true;
            break;
        }
    }
    vector<string>words;//to store all the labels, operands,mnemonics
    vector<int>words_len;//getting each of the words length
    stringstream x(k);//string stream of that respective line
    string buffer;//to store each of the words

    while(x >> buffer)//storing each element
    {
        words.push_back(buffer);
        source_code[(*pc)].push_back(buffer);
        words_len.push_back(buffer.length());
    }
    
    if(label)//label exists
    {
        string label_name = words[0];//assuming the first words of label exists is the label only

        if(!(isValidLabel(label_name)))//invalid label
        {
        errors.push_back({line,"Invalid label name it should contain only alphanumeric or underscore symbol and should begin with an alphadet or underscore"});
        if(words.size()!=1)
                (*pc)++;
        }
        else//valid label
        {
            if(labels.find(label_name)!=labels.end())//duplicate label
            {
                errors.push_back({line,"Label already exists, duplicate label defnition"});
                if(words.size()!=1)
                (*pc)++;
            }
            else//unique label
            {
                labels.insert({label_name,*pc});
                labels_line.insert({label_name,line});
                if(words.size()==1)//only label, then its pc and value stored, move on
                {
                    return;
                }
                else//check following instructions
                {
                    string instructions = words[1];//checks to see if mnemonic is valid or not along with respective operands

                    if(inst_0_op.find(instructions)!= inst_0_op.end())//if corresponds to proper 0operand instruction
                    {
                        if(words.size()!=2)
                            errors.push_back({line,"No operand is allowed"});

                    }
                    else if(inst_1_op.find(instructions)!=inst_1_op.end())//checks if one operand instruction is valid
                    {
                        if(words.size()>3)
                            errors.push_back({line,"Only one operand is allowed"});
                        else if(words.size()<3)
                            errors.push_back({line,"No operand"});

                    }
                    else
                    {
                        //Error that the instruction is invalid
                        errors.push_back({line,"Invalid Instruction"});
                    }
                }
                (*pc)++;
            }
        }
    }
    else//No label only instruction
    {
        string instructions = words[0];//checks to see if mnemonic is valid or not along with respective operands

                    if(inst_0_op.find(instructions)!= inst_0_op.end())//if corresponds to proper 0operand instruction
                    {
                        if(words.size()!=1)
                            errors.push_back({line,"No operand is allowed"});

                    }
                    else if(inst_1_op.find(instructions)!=inst_1_op.end())//checks if one operand instruction is valid
                    {
                        if(words.size()>2)
                            errors.push_back({line,"Only one operand is allowed"});
                        else if(words.size()<2)
                            errors.push_back({line,"No operand"});

                    }
                    else
                    {
                        //Error that the instruction is invalid
                        errors.push_back({line,"Invalid Instruction"});
                    }
        (*pc)++;
    }
}

string decl_to_hexa(int num,int no_bits)
{
    stringstream sss;
    sss << hex << setw(no_bits) << setfill('0') << num;//set width while converting the number in stream to hex

     string result = sss.str();
    if (result.size() > no_bits)//to handle -ve numbers
    {
        result = result.substr(result.size() - no_bits);
    }

    return result;
}

int convert_to_decimal(string s,int line)
{
    if(s=="0")
    return 0;
        string k=s;
    if(s[0]=='-' || s[0]=='+')//dealing with optional + or - signs
    {
         k = s.substr(1);
    }

    int value = 0;
    int ch = isValid(k);// calls the isVAlid function and then checks case wise
    switch(ch)
    {
        case 0:// Not a valid format
        {
           
            {
                errors.push_back({line,"(Line number here corresponds to the Listing file Line) Not a valid number format"});
                return 0;
            }
        }
        case 1://Wrong label is called
        {
            errors.push_back({line,"(Line number here corresponds to the Listing file Line) No such label"});
            return 0;
        }
        case 2://correct label
        {
            value = labels[s];
            return value;
        }
        case 6 ://returns the correct opcode
        {
         if(inst_0_op.find(s)!=inst_0_op.end())
            {
                value = inst_0_op[s];
                return value;
            }
        if(inst_1_op.find(s)!=inst_1_op.end())
            {
                value = inst_1_op[s];
                return value;
            }
        }
        default://deals the the conversion of a string in any of hex,octal or decimal to its integer 
        {
            char *endptr;
            long long value_2 = strtoll(s.c_str(),&endptr,0);
            value = (int)value_2;
            return value;
        }
    }
}

void SET()
{
    for(int i = 0; i<source_code.size();i++) //check for the occurence of SET
    {
        if(source_code[i].empty())
			break;

     int j = 0;
       for(j;j<source_code[i].size();j++)//Find first non label element
       {
        if(labels.find(source_code[i][j]) == labels.end())
        break;
       }
        if((source_code[i][j] == "SET" && labels.find(source_code[i][j-1]) != labels.end()) && (j-1)>=0)//set the label value in the map to the operand value
        {
            labels[source_code[i][j-1]] = convert_to_decimal(source_code[i][j+1],i);
        }

    }
}

void make_machine_code(int instruction_type,string mnemonic,string operand,int pc, string label_to_be_used)
{
     if(inst_0_op.find(mnemonic)==inst_0_op.find("HALT"))// check if halt has been used
       {
        halt_flag=true;
       }
//For each possible instruction, call conversion of string to decimal and decimal to hex and store it in the machine code vector
    int value;
	if(instruction_type == 0)
	{
		machine_code_obj[pc] = inst_0_op[mnemonic];
		machine_code[pc] = decl_to_hexa(inst_0_op[mnemonic],8);
	}
	else if(instruction_type == 1)
	{	
		if(mnemonic == "data")
		{
			used_labels.insert(label_to_be_used);
			value = convert_to_decimal(operand,pc);
			machine_code_obj[pc] = value;
			machine_code[pc] = decl_to_hexa(value,8);
		}
		else if(mnemonic == "SET")
		{
			used_labels.insert(label_to_be_used);
			value = convert_to_decimal(operand,pc);
			machine_code_obj[pc] = value;
			machine_code[pc] = decl_to_hexa(value,8);
		}
		else
		{
			int value2 = convert_to_decimal(mnemonic,pc);
			int value1 = convert_to_decimal(operand,pc);
			if( inst_1_of.find(mnemonic) != inst_1_of.end() )
			{
				if(isValidLabel(operand))
					value1 = (value1 - pc) - 1;								
			}
			machine_code_obj[pc] = value1<<8 | value2;
			machine_code[pc] = decl_to_hexa(value1,6) + decl_to_hexa(value2,2);
		}
	}   
}

void second_pass()//For every type of instruction identify type, figure out if theres an error and find corresponding machine code
{
    for(int i = 0; i<source_code.size(); i++)
    {
        if(source_code[i].empty())
        continue;

        if(source_code[i].size() == 1)
		{
			make_machine_code(0,source_code[i][0],"",i,"");
			continue;
		}
        string mnemonic,operand;
        int value = 0;
       int j = 0;
       for(j;j<source_code[i].size();j++)
       {
        if(labels.find(source_code[i][j]) == labels.end())
        break;
       }
       mnemonic = source_code[i][j];
       if(inst_0_op.find(mnemonic)!=inst_0_op.end())
       {
        make_machine_code(0,mnemonic,"",i,"");
       }
       else if(inst_1_op.find(mnemonic)!=inst_1_op.end())
       {
        operand = source_code[i][j+1];

        if(labels.find(operand)!=labels.end())
        {
            used_labels.insert(operand);
        make_machine_code(1,mnemonic,operand,i,source_code[i][0]);
       }
       else
       {
        make_machine_code(1,mnemonic,operand,i,"");
       }
    }
}
}

void identify_warnings()//Used to identify the warnings
{
    for(auto it = labels.begin();it!=labels.end();it++)
    {
        string s = it->first;
            if(used_labels.find(s)==used_labels.end())
        {
            int line = labels_line[s];
            warnings.push_back({line,"Unused label"});
        }
    }
    if(!halt_flag)
        warnings.push_back({0,"Missing Halt"});
}

void print_warnings()
{
	cout << "Warnings : (Line 0 means unidentified location) \n";
	for(auto it=warnings.begin();it!=warnings.end();it++)
	{
		cout << "Line " << it->first << " : " << it->second << "\n";
	}
	cout << "\n";
}

void print_errors()
{
	cout << "Errors : (Line 0 means unidentified location) \n";
	for(auto it=errors.begin();it!=errors.end();it++)
	{
		cout << "Line " << it->first << " : " << it->second << "\n";
	}
	cout << "\n";
}


void generate_log()
{
	string s = fileName;
	s += ".log";
	fstream log_fptr;
	log_fptr.open(s,ios::out);

    if(log_fptr.is_open())
	{
		log_fptr << "LabelName => Value \n"; 
		for(auto x:labels)
		{
			log_fptr << x.first << " => " << x.second << "\n";
		}

		log_fptr << "\nUsedLabelName\n"; 
		for(auto x:used_labels)
		{
			if(x != "")
				log_fptr << x << "\n";
		}

		log_fptr << "\nWarnings \n";
		for(auto x:warnings)
		{
			log_fptr << "Line " << x.first << " : " << x.second << "\n";
		}

		log_fptr << "\nErrors \n";
		for(auto x:errors)
		{
			log_fptr << "Line " << x.first << " : " << x.second << "\n";
		}
	}
	log_fptr.close();
	return;
}

void generate_list()
{
	fstream list_fptr;
	string s = fileName;
	s += ".lst";
	list_fptr.open(s,ios::out);

	if(list_fptr.is_open())
	{
		for(int i=0;i<source_code.size();i++)
		{
			if(source_code[i].empty())
				break;

			list_fptr << decl_to_hexa(i,8) << " ";
			list_fptr << machine_code[i] << " ";

			for(int j=0;j<source_code[i].size();j++)
			{
				list_fptr << source_code[i][j] << " ";
			}
			list_fptr << "\n";
		}
	}
	list_fptr.close();
	return;
}

void generate_obj()
{
	FILE* obj_fptr;
	string s = fileName;
	s += ".o";
	obj_fptr = fopen(s.c_str(),"wb");
	int i = 0;
	for(i;i<source_code.size();i++)
	{
 		if(source_code[i].empty())
			break;
		i++;
	}
	fwrite(machine_code_obj,sizeof(int),i,obj_fptr);
	fclose(obj_fptr);
	return;
}





