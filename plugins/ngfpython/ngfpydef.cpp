#include <iostream>
#include <sstream>
#include <string>

using namespace std;

string type;
string pyname;

void parseLine(string str)
{
    type = (pyname = "");
    const char *cStr = str.c_str();

    while ((*cStr == ' ') || (*cStr == '\t'))
    {
	++cStr;
    }

    if ((*cStr == '}') ||(*cStr == '{') || (*cStr == '\0'))
    {
	return;
    }

    if (*cStr == '\n')
	return;

    do
    {
	type += *cStr;
    } while (*++cStr && *cStr != '(');

    ++cStr;
    
    do
    {
	pyname += *cStr;
    } while (*++cStr && *cStr != ')');
}

int main(int argc, char *argv[])
{
    stringstream outstream(stringstream::in | stringstream::out);
    string instr, classname;

    outstream <<
    "%{\n"
    "#ifndef __PYTHON_METHOD_STRUCT__\n"
    "#define __PYTHON_METHOD_STRUCT__\n"
    "%}\n";
    	
    while(getline(cin, instr))
    {
    	parseLine(instr);
	if (type == "NGF_PY_BEGIN_DECL")
	{
	    outstream << "%define class-name NGF_PY_CLASS_GPERF(" + pyname + ")\n\n";
	    classname = pyname;
	    break;
	}
    }

    outstream <<
    "%define hash-function-name MakeHash\n"
    "%define lookup-function-name Lookup\n"
    "%language=C++\n"
    "%readonly-tables\n"
    //"%switch=1\n"
    "%enum\n"
    "%struct-type\n"
    "struct PythonMethod\n"
    "{\n"
	"const char *name;\n"
	"int code;\n"
    "};\n"
    "#endif //\n"
    "\n"
    "%%\n";

    while (getline(cin, instr))
    {
	parseLine(instr);

	if (type == "NGF_PY_METHOD_DECL")
	{
	    outstream << pyname + ", NGF_PY_METHOD_GPERF(" + classname + ", " + pyname + ")\n";
	}
	else if (type == "NGF_PY_PROPERTY_DECL")
	{
	    outstream << "get_" + pyname + ", NGF_PY_GET_GPERF(" + classname + ", " + pyname + ")\n";
	    outstream << "set_" + pyname + ", NGF_PY_SET_GPERF(" + classname + ", " + pyname + ")\n";
	}
    }

    cout << outstream.str();

    return 0;
}
