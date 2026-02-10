/*
CppOpenScraper - C++ Web Scraping Library
Copyright (C) 2025 Adrian Maire (escain@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LEXBOR_DOCUMENT_H
#define LEXBOR_DOCUMENT_H

#include <string>
#include <vector>

struct lxb_html_document;
struct lxb_dom_node;

namespace CppScrap
{

class LexborDocument final
{
public:
	using Node = lxb_dom_node*;

	/// @brief Create an empty lexbor HTML document.
	LexborDocument();
	/// @brief Destroy the lexbor document and free resources.
	~LexborDocument();

	LexborDocument(const LexborDocument&) = delete;
	auto operator=(const LexborDocument&) -> LexborDocument& = delete;

	/// @brief Parse an HTML string into a DOM tree.
	auto parse(const std::string& html) -> bool;
	/// @brief Return the document title from the <title> tag.
	auto title() const -> std::string;

	/// @brief Return the <body> element node.
	auto body() const -> Node;
	/// @brief Return the lowercase tag name of an element node.
	auto tagName(Node node) const -> std::string;
	/// @brief Get an attribute value by name from an element.
	auto attribute(Node node, const std::string& name) const -> std::string;
	/// @brief Return the element's id attribute value.
	auto id(Node node) const -> std::string;
	/// @brief Return the element's class attribute value.
	auto className(Node node) const -> std::string;
	/// @brief Recursively extract all text content from a node subtree.
	auto textContent(Node node) const -> std::string;
	/// @brief Return the first child node.
	auto firstChild(Node node) const -> Node;
	/// @brief Return the next sibling node.
	auto nextSibling(Node node) const -> Node;
	/// @brief Return the parent node.
	auto parent(Node node) const -> Node;
	/// @brief Check if a node is a DOM element.
	auto isElement(Node node) const -> bool;
	/// @brief Check if a node is a text node.
	auto isText(Node node) const -> bool;

	/// @brief Detach and destroy a node and its entire subtree.
	auto removeNode(Node node) -> void;

	/// @brief Find all descendant elements matching a tag name.
	auto findByTag(Node root, const std::string& tag) const -> std::vector<Node>;

private:
	lxb_html_document* m_document = nullptr;
};

}

#endif // LEXBOR_DOCUMENT_H
