#include <boost/assign/list_inserter.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <mysqlxx/String.h>

#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/DataTypes/DataTypeDate.h>
#include <DB/DataTypes/DataTypeDateTime.h>
#include <DB/DataTypes/DataTypeString.h>
#include <DB/DataTypes/DataTypeFixedString.h>
#include <DB/DataTypes/DataTypeAggregateFunction.h>
#include <DB/DataTypes/DataTypeArray.h>
#include <DB/DataTypes/DataTypeNested.h>
#include <DB/DataTypes/DataTypeFactory.h>

#include <DB/AggregateFunctions/AggregateFunctionFactory.h>

#include <DB/Parsers/ExpressionListParsers.h>
#include <DB/Parsers/ParserCreateQuery.h>
#include <DB/Parsers/ASTExpressionList.h>
#include <DB/Parsers/ASTNameTypePair.h>

namespace DB
{


DataTypeFactory::DataTypeFactory()
	: fixed_string_regexp("^FixedString\\s*\\(\\s*(\\d+)\\s*\\)$"),
	nested_regexp("^(\\w+)\\s*\\(\\s*(.+)\\s*\\)$", Poco::RegularExpression::RE_MULTILINE | Poco::RegularExpression::RE_DOTALL)
{
	boost::assign::insert(non_parametric_data_types)
		("UInt8",				new DataTypeUInt8)
		("UInt16",				new DataTypeUInt16)
		("UInt32",				new DataTypeUInt32)
		("UInt64",				new DataTypeUInt64)
		("Int8",				new DataTypeInt8)
		("Int16",				new DataTypeInt16)
		("Int32",				new DataTypeInt32)
		("Int64",				new DataTypeInt64)
		("Float32",				new DataTypeFloat32)
		("Float64",				new DataTypeFloat64)
		("Date",				new DataTypeDate)
		("DateTime",			new DataTypeDateTime)
		("String",				new DataTypeString)
		;
}


DataTypePtr DataTypeFactory::get(const String & name) const
{
	NonParametricDataTypes::const_iterator it = non_parametric_data_types.find(name);
	if (it != non_parametric_data_types.end())
		return it->second;

	Poco::RegularExpression::MatchVec matches;
	if (fixed_string_regexp.match(name, 0, matches) && matches.size() == 2)
		return new DataTypeFixedString(mysqlxx::String(name.data() + matches[1].offset, matches[1].length, NULL).getUInt());

	if (nested_regexp.match(name, 0, matches) && matches.size() == 3)
	{
		String base_name(name.data() + matches[1].offset, matches[1].length);
		String parameters(name.data() + matches[2].offset, matches[2].length);

		if (base_name == "Array")
			return new DataTypeArray(get(parameters));
		
		if (base_name == "AggregateFunction")
		{
			String function_name;
			AggregateFunctionPtr function;
			DataTypes argument_types;

			ParserExpressionList args_parser;
			ASTPtr args_ast;
			String expected;
			IParser::Pos pos = parameters.data();
			IParser::Pos end = pos + parameters.size();

			if (!(args_parser.parse(pos, end, args_ast, expected) && pos == end))
				throw Exception("Cannot parse parameters for data type " + name, ErrorCodes::SYNTAX_ERROR);

			ASTExpressionList & args_list = dynamic_cast<ASTExpressionList &>(*args_ast);
			
			if (args_list.children.empty())
				throw Exception("Data type AggregateFunction requires parameters: "
					"name of aggregate function and list of data types for arguments", ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH);

			ASTs::iterator it = args_list.children.begin();
			function_name = (*it)->getColumnName();

			for (++it; it != args_list.children.end(); ++it)
				argument_types.push_back(get((*it)->getColumnName()));

			function = AggregateFunctionFactory().get(function_name, argument_types);
			return new DataTypeAggregateFunction(function, argument_types);
		}
		
		if (base_name == "Nested")
		{
			ParserNameTypePairList columns_p;
			ASTPtr columns_ast;
			String expected;
			IParser::Pos pos = parameters.data();
			IParser::Pos end = pos + parameters.size();
			
			if (!(columns_p.parse(pos, end, columns_ast, expected) && pos == end))
				throw Exception("Cannot parse parameters for data type " + name, ErrorCodes::SYNTAX_ERROR);
			
			NamesAndTypesListPtr columns = new NamesAndTypesList;

			ASTExpressionList & columns_list = dynamic_cast<ASTExpressionList &>(*columns_ast);
			for (ASTs::iterator it = columns_list.children.begin(); it != columns_list.children.end(); ++it)
			{
				ASTNameTypePair & name_and_type_pair = dynamic_cast<ASTNameTypePair &>(**it);
				StringRange type_range = name_and_type_pair.type->range;
				columns->push_back(NameAndTypePair(
					name_and_type_pair.name,
					get(String(type_range.first, type_range.second - type_range.first))));
			}
			
			return new DataTypeNested(columns);
		}
		
		throw Exception("Unknown type " + base_name, ErrorCodes::UNKNOWN_TYPE);
	}

	throw Exception("Unknown type " + name, ErrorCodes::UNKNOWN_TYPE);
}


}
