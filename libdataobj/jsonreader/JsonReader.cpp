#include "JsonReader.h"
#include "../DataObject.h"
#include "../Exception.h"
#include "JsonNodeProcessor.h"
#include "StringProcessor.h"
#include "IntegerProcessor.h"
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace std;
using namespace dataobject::jsonreader;
using namespace dataobject::jsonreader::processors;

namespace dataobject::jsonreader
{

JsonNodeProcessor* JsonReader::detectJsonNode(const char& _ch)
{
    if (_ch == '"')
        return new StringProcessor();
    if (_ch == '{')
        return new ObjectProcessor();
    if (_ch == '[')
        return new ArrayProcessor();
    if (std::isdigit(_ch))
        return  new IntegerProcessor(_ch);
    throw DataObjectException(string("Undetermend processor ") + _ch);
    return nullptr;
}


void JsonReader::processLine(string const& _line)
{
    (void)m_stopper;
    m_processedLineNumber++;
    // Reading root json object from file. The root json must include one object.
    if (_line.empty())
        return;
    try
    {
        if (m_processor->finalized())
            throw DataObjectException("JsonReader::processLine: Unexpected json file end on line: " + _line);
        for (auto const& ch : _line)
        {
            m_processor->processChar(ch);
        }
    }
    catch (DataObjectException const& _ex)
    {
        throw DataObjectException(string(_ex.what()) + "\n Line " + std::to_string(m_processedLineNumber) + ": " + _line);
    }
}
}  // namespace dataobject::jsonreader

namespace dataobject
{
spDataObject ConvertJsoncppFileToData(string const& _file, string const& _stopper, bool _autosort)
{
    std::ifstream file(_file);
    if (file.is_open())
    {
        string line;
        JsonReader reader(_stopper, _autosort);
        //size_t k = 0;
        while (std::getline(file, line))
        {
            reader.processLine(line);

            //if (k++ == 100)
            //    break;
        }
        file.close();

        if (!reader.finalized())
            throw DataObjectException("ConvertJsoncppFileToData: Json structure incomplete in file: " + _file);

        std::cerr << reader.getResult()->asJson() << std::endl;
        return reader.getResult();
    }
    else
        throw DataObjectException() << "ConvertJsoncppFileToData: can't open file: `" + _file;
}

}  // namespace dataobject
