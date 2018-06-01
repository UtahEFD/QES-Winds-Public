#include "ParseInterface.h"
#include "Root.h"

	ParseInterface::ParseInterface()
	{
		root = 0;
	}

	ParseInterface::ParseInterface(pt::ptree t)
	{
		root = new Root();
		tree = t;
	}

	template <typename T>
	void ParseInterface::parsePrimative(T& val, const std::string tag)
	{
		boost::optional<T> newVal = tree.get_optional<T>(tag);
		if (newVal)
			val = *newVal;
	}

	template <typename T>
	void ParseInterface::parseMultiPrimatives(std::vector<T>& vals, const std::string tag)
	{
		pt::ptree::const_iterator end = tree.end();
		for (pt::ptree::const_iterator it = tree.begin(); it != end; ++it)
		{
			if (it->first == tag)
			{
				T newVal;
				newVal = (it->second).get_value<T>();
				vals.push_back(newVal);
			}
		}
	}

	template <typename T>
	void ParseInterface::parseElement(T*& ele, const std::string tag)
	{
		auto child = tree.get_child_optional(tag);
		if (child)
		{
			ele = new T();
			ele->setTree(*child);
			ele->parseValues();
		}
	}

	template <typename T>
	void ParseInterface::parseMultiElements(std::vector<T*>& eles, const std::string tag)
	{
		pt::ptree::const_iterator end = tree.end();
		for (pt::ptree::const_iterator it = tree.begin(); it != end; ++it)
		{
			if (it->first == tag)
			{
				T* newEle = new T();
				newEle->setTree(it->second);
				newEle->parseValues();
				eles.push_back(newEle);
			}
		}
	}

	template <typename T, typename X, typename... ARGS>
	void ParseInterface::parsePolymorph(T*& ele, X poly, ARGS... args)
	{
		auto child = tree.get_child_optional(poly.tag);
		if (child)
		{
			poly.setNewType(ele);
			ele->setTree(*child);
			ele->parseValues();
		}
		else
			parsePolymorph(ele, args...);
	}


	template <typename T, typename X>
	void ParseInterface::parsePolymorph(T*& ele, X poly)
	{
		auto child = tree.get_child_optional(poly.tag);
		if (child)
		{
			poly.setNewType(ele);
			ele->setTree(*child);
			ele->parseValues();
		}
	}


	template <typename T, typename X, typename... ARGS>
	void ParseInterface::parseMultiPolymorphs(std::vector<T*>& eles, X poly, ARGS... args)
	{
		pt::ptree::const_iterator end = tree.end();
		for (pt::ptree::const_iterator it = tree.begin(); it != end; ++it)
		{
			if (it->first == poly.tag)
			{
				T* newEle;
				poly.setNewType(newEle);
				newEle->setTree(it->second);
				newEle->parseValues();
				eles.push_back(newEle);
			}
		}
		parseMultiPolymorphs(eles, args...);
	}


	template <typename T, typename X>
	void ParseInterface::parseMultiPolymorphs(std::vector<T*>& eles, X poly)
	{
		pt::ptree::const_iterator end = tree.end();
		for (pt::ptree::const_iterator it = tree.begin(); it != end; ++it)
		{
			if (it->first == poly.tag)
			{
				T* newEle;
				poly.setNewType(newEle);
				newEle->setTree(it->second);
				newEle->parseValues();
				eles.push_back(newEle);
			}
		}
	}


	void ParseInterface::parseValues() 
	{
		root->setTree(tree);
		root->parseValues();
	}
