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

#include "LexborDocument.hpp"

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>
#include <lexbor/dom/interfaces/element.h>

namespace CppScrap
{

LexborDocument::LexborDocument()
	: m_document(lxb_html_document_create())
{
}

LexborDocument::~LexborDocument()
{
	if (m_document)
	{
		lxb_html_document_destroy(m_document);
	}
}

auto LexborDocument::parse(const std::string& html) -> bool
{
	if (!m_document)
	{
		return false;
	}

	auto status = lxb_html_document_parse(
		m_document,
		reinterpret_cast<const lxb_char_t*>(html.data()),
		html.size());

	return status == LXB_STATUS_OK;
}

auto LexborDocument::title() const -> std::string
{
	if (!m_document)
	{
		return {};
	}

	size_t len = 0;
	const auto* t = lxb_html_document_title(m_document, &len);
	if (!t || len == 0)
	{
		return {};
	}

	return {reinterpret_cast<const char*>(t), len};
}

auto LexborDocument::body() const -> Node
{
	if (!m_document)
	{
		return nullptr;
	}

	auto* bodyElem = lxb_html_document_body_element(m_document);
	if (!bodyElem)
	{
		return nullptr;
	}

	return lxb_dom_interface_node(bodyElem);
}

auto LexborDocument::tagName(Node node) const -> std::string
{
	if (!node || !isElement(node))
	{
		return {};
	}

	auto* elem = lxb_dom_interface_element(node);
	size_t len = 0;
	const auto* name = lxb_dom_element_local_name(elem, &len);
	if (!name || len == 0)
	{
		return {};
	}

	return {reinterpret_cast<const char*>(name), len};
}

auto LexborDocument::attribute(Node node, const std::string& name) const -> std::string
{
	if (!node || !isElement(node))
	{
		return {};
	}

	auto* elem = lxb_dom_interface_element(node);
	size_t valueLen = 0;
	const auto* val = lxb_dom_element_get_attribute(
		elem,
		reinterpret_cast<const lxb_char_t*>(name.data()),
		name.size(),
		&valueLen);

	if (!val || valueLen == 0)
	{
		return {};
	}

	return {reinterpret_cast<const char*>(val), valueLen};
}

auto LexborDocument::id(Node node) const -> std::string
{
	if (!node || !isElement(node))
	{
		return {};
	}

	auto* elem = lxb_dom_interface_element(node);
	size_t len = 0;
	const auto* val = lxb_dom_element_id(elem, &len);
	if (!val || len == 0)
	{
		return {};
	}

	return {reinterpret_cast<const char*>(val), len};
}

auto LexborDocument::className(Node node) const -> std::string
{
	if (!node || !isElement(node))
	{
		return {};
	}

	auto* elem = lxb_dom_interface_element(node);
	size_t len = 0;
	const auto* val = lxb_dom_element_class(elem, &len);
	if (!val || len == 0)
	{
		return {};
	}

	return {reinterpret_cast<const char*>(val), len};
}

auto LexborDocument::textContent(Node node) const -> std::string
{
	if (!node)
	{
		return {};
	}

	size_t len = 0;
	auto* text = lxb_dom_node_text_content(node, &len);
	if (!text || len == 0)
	{
		return {};
	}

	std::string result(reinterpret_cast<const char*>(text), len);

	// lexbor allocates this string, we must free it
	lxb_dom_document_t* doc = &m_document->dom_document;
	lexbor_mraw_free(doc->text, text);

	return result;
}

auto LexborDocument::firstChild(Node node) const -> Node
{
	if (!node)
	{
		return nullptr;
	}

	return lxb_dom_node_first_child(node);
}

auto LexborDocument::nextSibling(Node node) const -> Node
{
	if (!node)
	{
		return nullptr;
	}

	return lxb_dom_node_next(node);
}

auto LexborDocument::parent(Node node) const -> Node
{
	if (!node)
	{
		return nullptr;
	}

	return lxb_dom_node_parent(node);
}

auto LexborDocument::isElement(Node node) const -> bool
{
	return node && node->type == LXB_DOM_NODE_TYPE_ELEMENT;
}

auto LexborDocument::isText(Node node) const -> bool
{
	return node && node->type == LXB_DOM_NODE_TYPE_TEXT;
}

auto LexborDocument::removeNode(Node node) -> void
{
	if (!node)
	{
		return;
	}

	lxb_dom_node_remove(node);
	lxb_dom_node_destroy_deep(node);
}

auto LexborDocument::findByTag(Node root, const std::string& tag) const -> std::vector<Node>
{
	std::vector<Node> results;

	if (!root || !m_document)
	{
		return results;
	}

	auto* collection = lxb_dom_collection_make(&m_document->dom_document, 16);
	if (!collection)
	{
		return results;
	}

	auto* elem = lxb_dom_interface_element(root);
	if (elem)
	{
		lxb_dom_elements_by_tag_name(
			elem,
			collection,
			reinterpret_cast<const lxb_char_t*>(tag.data()),
			tag.size());

		auto len = lxb_dom_collection_length(collection);
		results.reserve(len);
		for (size_t i = 0; i < len; ++i)
		{
			results.push_back(lxb_dom_collection_node(collection, i));
		}
	}

	lxb_dom_collection_destroy(collection, true);

	return results;
}

}
