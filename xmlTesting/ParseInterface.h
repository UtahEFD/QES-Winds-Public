#pragma once

#include <string>

#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <iostream>

#include <vector>

class Root;

namespace pt = boost::property_tree;

/**
 * This structure is for grouping information necessary
 * for parsing out polymorphic objects from an XML. This takes
 * two template types, one for the base class, and one for the
 * inheriting class. The constructor takes in a string which
 * is used as a tag for finding values in the XML
 */

template<typename generic, typename thisType>
struct Polymorph
{
	typedef thisType T;
	typedef generic G;
	std::string tag;

	Polymorph(std::string t)
	{
		tag = t;
	}

	void setNewType(G*& element)
	{
		element = new T();
	}
};


/**
 * This class is a generic object from which all classes that can be parsed from an XML 
 * file will inherit from. This class contains methods to simplify the parsing process
 * and obscure the complecations of the boost library.
 */

class ParseInterface
{
private:

protected:
	pt::ptree tree;

	 /**
	 * This sets the tree of this object
	 */
	void setTree(pt::ptree t) { tree = t;}

public:
	Root* root;
	/**
	 * default constructor
	 */
	ParseInterface();

	/**
	 * sets root to null, takes in a default tree
	 */
	ParseInterface(pt::ptree t);

	/**
	 * This function parses the current node of the tree and searches for an element
	 * with the tag of "tag". Once it is found, it returns the value.
	 * this is a template function, so the type of the value that is taken in and
	 * found are flexible. If the value is unable to be found, "val" will not be
	 * updated.
	 * @param val the value that is being updated
	 * @param tag the tagline in the xml of the value we are searching for
	 */
	template <typename T>
	void parsePrimative(T& val, const std::string tag);

	/**
	 * This function parses the current node of the tree and searches for all elements
	 * with the tag of "tag". When one is found, it adds the value to a vector.
	 * this is a template function, so the type of the value that is taken in and
	 * found are flexible. This will return all instances of a certain tag, this means it
	 * can have 0..* values.
	 * @param vals the vector to be filled with found values
	 * @param tag the tagline in the xml of the values we are searching for
	 */
	template <typename T>
	void parseMultiPrimatives(std::vector<T>& vals, const std::string tag);


	/**
	 * This function parses the current node of the tree and searches for an element
	 * with the tag of "tag". Once it is found, that element will be 
	 * parsed and the completed element will be returned.
	 * this is a template function, so the type of the value that is taken in and
	 * found are flexible. If the value is unable to be found, "val" will not be
	 * updated.
	 * @param ele the element that is being parsed and updated
	 * @param tag the tagline in the xml of the element we are searching for
	 */
	template <typename T>
	void parseElement(T*& ele, const std::string tag);


	/**
	 * This function parses the current node of the tree and searches for all elements
	 * with the tag of "tag". Once it is found, that element will be 
	 * parsed and the completed element will be returned.
	 * this is a template function, so the type of the value that is taken in and
	 * found are flexible. This will return all instances of a certain tag, this means it
	 * can have 0..* elements.
	 * @param eles the vector to be filled with found elements
	 * @param tag the tagline in the xml of the elements we are searching for
	 */
	template <typename T>
	void parseMultiElements(std::vector<T*>& eles, const std::string tag);


	/**
	 * Recursive
	 * This function takes in an element and a list of polymorphs. This function
	 * will check the current ptree for all of the provided polymorphs. Once one
	 * of the polymorphs is found, an element of that type is created over ele and
	 * it's information is parsed out of the tree.
	 * This function is a variadic template, which means it can take in any number
	 * of objects each with their own type. However, this code is expecting each 
	 * type to be a variation of a polymorph and as so, this calls functions and
	 * uses variables specific to the polymorph structure.
	 * @param ele the element that will be updated
	 * @param poly the current polymorph we are looking for in the xml
	 * @param ARGS subsequent polymorphs to check in case the current is not found
	 */
	template <typename T, typename X, typename... ARGS>
	void parsePolymorph(T*& ele, X poly, ARGS... args);

	/**
	 * A base case for the recursive version of this function
	 */
	template <typename T, typename X>
	void parsePolymorph(T*& ele, X poly);

	/**
	 * Recursive
	 * This function takes in a vector of elements and a list of polymorphs. This function
	 * will check the current ptree for all of the provided polymorphs. All instances
	 * of the polymorphs in the current tree are created through parsing and then added
	 * to the vector.
	 * This function is a variadic template, which means it can take in any number
	 * of objects each with their own type. However, this code is expecting each 
	 * type to be a variation of a polymorph and as so, this calls functions and
	 * uses variables specific to the polymorph structure.
	 * @param eles a list of elements that will be updated
	 * @param poly the current polymorph we are looking for in the xml
	 * @param ARGS subsequent polymorphs to check in case the current is not found
	 */
	template <typename T, typename X, typename... ARGS>
	void parseMultiPolymorphs(std::vector<T*>& eles, X poly, ARGS... args);

	/**
	 * A base case for the recursive version of this function
	 */
	template <typename T, typename X>
	void parseMultiPolymorphs(std::vector<T*>& eles, X poly);

	/**
	 * Virtual function
	 * This is where we select what values we want to parse out of
	 * the xml file by calling the parseX functions above.
	 */
	virtual void parseValues();

	/**
	 * This function returns the root value
	 */
	Root* getRoot() { return root; }


};