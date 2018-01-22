#ifndef _C4_RYML_HPP_
#include <ryml/ryml.hpp>
#endif

#include <ctype.h>

namespace c4 {
namespace yml {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

size_t Node::id() const
{
    return m_s->id(this);
}

const char* Node::type_str(NodeType_e ty)
{
    switch(ty)
    {
    case VAL     : return "VAL";
    case MAP     : return "MAP";
    case SEQ     : return "SEQ";
    case KEYVAL  : return "KEYVAL";
    case KEYMAP  : return "KEYMAP";
    case KEYSEQ  : return "KEYSEQ";
    case DOC     : return "DOC";
    case DOCSEQ  : return "DOCSEQ";
    case DOCMAP  : return "DOCMAP";
    case STREAM  : return "STREAM";
    case NOTYPE  : return "NOTYPE";
    default: return "(unknown?)";
    }
}

Node * Node::parent() const
{
    return m_s->get(m_parent);
}

size_t Node::num_children() const
{
    if(is_val()) return 0;
    size_t count = 0;
    for(Node const* n = m_s->get(m_children.first); n; n = n->next_sibling())
    {
        ++count;
    }
    return count;
}

Node * Node::child(size_t i) const
{
    if(is_val()) return nullptr;
    size_t count = 0;
    for(Node *n = m_s->get(m_children.first); n; n = n->next_sibling(), ++count)
    {
        if(count == i) return n;
    }
    C4_ASSERT(i < count);
    return nullptr;
}
Node * Node::first_child() const
{
    if(is_val()) return nullptr;
    return m_s->get(m_children.first);
}
Node * Node::last_child() const
{
    if(is_val()) return nullptr;
    return m_s->get(m_children.last);
}

Node * Node::find_child(cspan const& name) const
{
    if(is_val()) return nullptr;
    C4_ASSERT(is_map());
    C4_ASSERT(bool(name) == true);
    if(m_children.first == NONE)
    {
        C4_ASSERT(m_children.last == NONE);
        return nullptr;
    }
    else
    {
        C4_ASSERT(m_children.last != NONE);
    }
    for(Node *n = m_s->get(m_children.first); n; n = n->next_sibling())
    {
        if(n->m_key == name)
        {
            return n;
        }
    }
    return nullptr;
}

bool Node::has_child(Node const* ch) const
{
    for(Node const* n = first_child(); n; n = n->next_sibling())
    {
        if(n == ch) return true;
    }
    return false;
}

size_t Node::num_siblings() const
{
    return (m_parent != NONE) ? m_s->get(m_parent)->num_children() : 0;
}

Node * Node::sibling(size_t i) const
{
    C4_ASSERT(parent() != nullptr);
    return m_s->get(m_parent)->child(i);
}

Node * Node::first_sibling() const
{
    auto p = parent();
    C4_ASSERT(p || is_root());
    if( ! p) return nullptr;
    return p->first_child();
}

Node * Node::last_sibling() const
{
    auto p = parent();
    C4_ASSERT(p || is_root());
    if( ! p) return nullptr;
    return p->last_child();
}

Node * Node::find_sibling(cspan const& name) const
{
    return m_s->get(m_parent)->find_child(name);
}

Node * Node::prev_sibling() const
{
    if(m_list.prev == NONE) return nullptr;
    if(m_s->id(this) == m_s->get(m_parent)->m_children.first) return nullptr;
    Node *n = m_s->get(m_list.prev);
    C4_ASSERT( ! n || n->m_parent == m_parent);
    return n;
}

Node * Node::next_sibling() const
{
    if(m_list.next == NONE) return nullptr;
    if(m_s->id(this) == m_s->get(m_parent)->m_children.last) return nullptr;
    Node *n = m_s->get(m_list.next);
    C4_ASSERT( ! n || n->m_parent == m_parent);
    return n;
}

bool Node::has_sibling(Node const* s) const
{
    for(Node *n = first_sibling(); n; n = n->next_sibling())
    {
        if(n == s) return true;
    }
    return false;
}

void Node::remove_child(cspan const& name)
{
    C4_ASSERT(find_child(name));
    Node *n = find_child(name);
    remove_child(n);
}

void Node::remove_child(size_t i)
{
    C4_ASSERT(is_container());
    C4_ASSERT(child(i));
    Node *n = child(i);
    remove_child(n);
}

void Node::remove_child(Node *n)
{
    C4_ASSERT(n && has_child(n));
    C4_ERROR("not implemented");
}


void Node::to_val(cspan const& val, int more_flags)
{
    C4_ASSERT( ! has_children());
    C4_ASSERT(m_parent == NONE || ! m_s->get(m_parent)->is_map());
    _set_flags(VAL|more_flags);
    m_key.clear();
    m_val = val;
}

void Node::to_keyval(cspan const& key, cspan const& val, int more_flags)
{
    C4_ASSERT( ! has_children());
    C4_ASSERT( ! key.empty());
    C4_ASSERT(m_parent != NONE && m_s->get(m_parent)->is_map());
    _set_flags(KEYVAL|more_flags);
    m_key = key;
    m_val = val;
}

void Node::to_map(int more_flags)
{
    C4_ASSERT( ! has_children());
    C4_ASSERT(m_parent == NONE || ! m_s->get(m_parent)->is_map());
    _set_flags(MAP|more_flags);
    m_key.clear();
    m_val.clear();
}

void Node::to_map(cspan const& key, int more_flags)
{
    C4_ASSERT( ! has_children());
    C4_ASSERT( ! key.empty());
    C4_ASSERT(m_parent != NONE && m_s->get(m_parent)->is_map());
    _set_flags(KEY|MAP|more_flags);
    m_key = key;
    m_val.clear();
}

void Node::to_seq(int more_flags)
{
    C4_ASSERT( ! has_children());
    _set_flags(SEQ|more_flags);
    m_key.clear();
    m_val.clear();
}

void Node::to_seq(cspan const& key, int more_flags)
{
    C4_ASSERT( ! has_children());
    C4_ASSERT(m_parent != NONE && m_s->get(m_parent)->is_map());
    _set_flags(KEY|SEQ|more_flags);
    m_key = key;
    m_val.clear();
}

void Node::to_doc(int more_flags)
{
    C4_ASSERT( ! has_children());
    _set_flags(DOC|more_flags);
    m_key.clear();
    m_val.clear();
}

void Node::to_stream(int more_flags)
{
    C4_ASSERT( ! has_children());
    _set_flags(STREAM|more_flags);
    m_key.clear();
    m_val.clear();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Tree::Tree()
    :
    m_buf(nullptr),
    m_cap(0),
    m_size(0),
    m_head(NONE),
    m_tail(NONE),
    m_free_head(NONE),
    m_free_tail(NONE)
{
}

Tree::Tree(size_t sz) : Tree()
{
    reserve(sz);
}

Tree::~Tree()
{
    if(m_buf)
    {
        RymlCallbacks::free(m_buf, m_cap * sizeof(Node));
    }
}

//-----------------------------------------------------------------------------
void Tree::reserve(size_t cap)
{
    if(cap <= m_cap) return;
    Node *buf = (Node*)RymlCallbacks::allocate(cap * sizeof(Node), nullptr);
    if(m_buf)
    {
        memcpy(buf, m_buf, m_cap * sizeof(Node));
        RymlCallbacks::free(m_buf, m_cap * sizeof(Node));
    }
    if(m_free_head == NONE)
    {
        C4_ASSERT(m_free_tail == m_free_head);
        m_free_head = m_cap;
        m_free_tail = cap;
    }
    else
    {
        C4_ASSERT(m_free_tail != NONE);
        m_buf[m_free_tail].m_list.next = m_cap;
    }
    size_t first = m_cap, del = cap - m_cap;
    m_cap = cap;
    m_buf = buf;
    clear_range(first, del);
    if( ! m_size)
    {
        claim(NONE);
        C4_ASSERT(id(root()) == 0);
    }
}

//-----------------------------------------------------------------------------
void Tree::clear()
{
    clear_range(0, m_cap);
    m_size = 0;
    m_head = NONE;
    m_tail = NONE;
    m_free_head = 0;
    m_free_tail = m_cap;
}

//-----------------------------------------------------------------------------
void Tree::clear_range(size_t first, size_t num)
{
    if(num == 0) return; // prevent overflow when subtracting
    C4_ASSERT(first >= 0 && first + num <= m_cap);
    memset(m_buf + first, 0, num * sizeof(Node));
    for(size_t i = first, e = first + num; i < e; ++i)
    {
        _clear(i);
        Node *n = m_buf + i;
        n->m_list.prev = i - 1;
        n->m_list.next = i + 1;
    }
    m_buf[first + num - 1].m_list.next = NONE;
}

//-----------------------------------------------------------------------------
size_t Tree::claim(size_t after)
{
    C4_ASSERT(after == NONE || (after >= 0 && after < m_cap));
    size_t last = after != NONE ? after : m_tail;
    size_t i = claim(last, NONE);
    return i;
}

//-----------------------------------------------------------------------------
size_t Tree::claim(size_t prev, size_t next)
{
    C4_ASSERT(prev == NONE || (prev >= 0 && prev < m_cap));
    C4_ASSERT(next == NONE || (next >= 0 && next < m_cap));
    if(m_free_head == NONE || m_buf == nullptr)
    {
        size_t sz = 2 * m_cap;
        sz = sz ? sz : 16;
        reserve(sz);
    }
    size_t id = m_free_head;
    Node *n = m_buf + id;
    m_free_head = n->m_list.next;
    if(m_free_head == NONE)
    {
        m_free_tail = NONE;
    }
    ++m_size;
    _clear(id);
    n->m_s = this;
    n->m_list.prev = prev;
    n->m_list.next = next;
    if(prev == NONE)
    {
        m_head = id;
    }
    else
    {
        Node *p = m_buf + prev;
        p->m_list.next = id;
    }
    if(next == NONE)
    {
        m_tail = id;
    }
    else
    {
        Node *v = m_buf + next;
        v->m_list.prev = id;
    }
    return id;
}

//-----------------------------------------------------------------------------
void Tree::release(size_t i)
{
    C4_ASSERT(i >= 0 && i < m_cap);
    Node & w = m_buf[i];

    // remove from the parent
    C4_ASSERT(w.m_parent != NONE || ! get(w.m_parent)->has_children());
    if(w.m_parent != NONE)
    {
        Node & p = m_buf[w.m_parent];
        if(p.m_children.first == i)
        {
            p.m_children.first = w.m_list.next;
        }
        if(p.m_children.last == i)
        {
            p.m_children.last = w.m_list.prev;
        }
    }

    /*
    // remove from the siblings
    if(w.m_siblings.prev != NONE)
    {
        Node &p = m_buf[w.m_siblings.prev];
        p.m_siblings.next = w.m_siblings.next;
    }
    if(w.m_siblings.next != NONE)
    {
        Node &n = m_buf[w.m_siblings.next];
        n.m_siblings.prev = w.m_siblings.prev;
    }
    w.m_siblings.next = NONE;
    w.m_siblings.prev = NONE;
    */

    // remove from the used list
    if(w.m_list.prev != NONE)
    {
        Node &p = m_buf[w.m_list.prev];
        p.m_list.next = w.m_list.next;
    }
    if(w.m_list.next != NONE)
    {
        Node &n = m_buf[w.m_list.next];
        n.m_list.prev = w.m_list.prev;
    }

    // add to the front of the free list
    w.m_list.next = m_free_head;
    w.m_list.prev = NONE;
    if(m_free_head != NONE)
    {
        m_buf[m_free_head].m_list.prev = i;
    }
    m_free_head = i;
    if(m_free_tail == NONE)
    {
        m_free_tail = m_free_head;
    }

    _clear(i);

    --m_size;
}

//-----------------------------------------------------------------------------
void Tree::set_parent(size_t iparent, size_t ichild, size_t iprev_sibling, size_t inext_sibling)
{
    Node *parent = get(iparent);
    Node *child = get(ichild);
    Node *prev_sibling = get(iprev_sibling);
    Node *next_sibling = get(inext_sibling);

    C4_ASSERT(child != nullptr && (child >= m_buf && child < m_buf + m_cap));
    C4_ASSERT(parent == nullptr || (parent >= m_buf && parent < m_buf + m_cap));

    child->m_parent = parent ? parent - m_buf : NONE;

    if( ! prev_sibling)
    {
        next_sibling = parent->first_child();
    }

    if( ! next_sibling)
    {
        if(prev_sibling)
        {
            next_sibling = prev_sibling->next_sibling();
        }
    }

    child->m_list.prev = NONE;
    child->m_list.next = NONE;
    if(prev_sibling)
    {
        C4_ASSERT(prev_sibling->next_sibling() == next_sibling);
        child->m_list.prev = prev_sibling->id();
        prev_sibling->m_list.next = child->id();
    }
    if(next_sibling)
    {
        C4_ASSERT(next_sibling->prev_sibling() == prev_sibling);
        child->m_list.next = next_sibling->id();
        next_sibling->m_list.prev = child->id();
    }

    if( ! parent) return;
    if(parent->m_children.first == NONE)
    {
        C4_ASSERT(parent->m_children.last == NONE);
        parent->m_children.first = child->id();
        parent->m_children.last = child->id();
    }
    else
    {
        if(child->m_list.next == parent->m_children.first)
        {
            parent->m_children.first = child->id();
        }
        if(child->m_list.prev == parent->m_children.last)
        {
            parent->m_children.last = child->id();
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// some debugging scaffolds

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC system_header

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

/** a debug utility */
/*
#define _prhl(fmt, ...)                                                 \
    {                                                                   \
        auto _llen = printf("%s:%d: line %zd:%zd(%zd): ",               \
                          __FILE__, __LINE__,                           \
                          m_state.pos.line-1,                           \
                          m_state.line_contents.rem.begin() - m_state.line_contents.stripped.begin(), \
                          m_state.line_contents.rem.len);               \
        if(m_state.line_contents.rem)                                   \
        {                                                               \
            printf("'%.*s'\n", _c4prsp(m_state.line_contents.rem));     \
        }                                                               \
        if(strlen(fmt) > 0)                                             \
        {                                                               \
            printf("%*s", _llen, " ");                                  \
            printf(fmt "\n", ## __VA_ARGS__);                           \
        }                                                               \
    }
*/
#ifdef RYML_DBG
#   define _c4prt_(fn, file, line, what, msg, ...)        \
    this->_##fn(file ":" C4_QUOTE(line) ": " what msg, ## __VA_ARGS__)
#   define _c4err(fmt, ...)  _c4prt_(err, "\n" __FILE__, __LINE__, "ERROR parsing yml: ", fmt, ## __VA_ARGS__)
#   define _c4dbgf(fmt, ...) _c4prt_(dbg,      __FILE__, __LINE__,                    "", fmt, ## __VA_ARGS__)
#   define _c4dbgp(fmt, ...)  printf(__FILE__ ":" C4_XQUOTE(__LINE__) ": " fmt "\n", ## __VA_ARGS__)
#   define _c4dbgs(fmt, ...)  printf(fmt "\n", ## __VA_ARGS__)
#else
#   define _c4err(fmt, ...) this->_err(fmt, ## __VA_ARGS__)
#   define _c4dbg(fmt, ...)
#   define _c4dbgs(fmt, ...)
#endif

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

//-----------------------------------------------------------------------------
Parser::Parser()
    :
    m_file(),
    m_buf(),
    m_root_id(NONE),
    m_tree(),
    m_stack(),
    m_state(),
    m_deindent()
{
    m_stack.reserve(16);
    m_stack.push({});
    m_state = &m_stack.top();
}

//-----------------------------------------------------------------------------
void Parser::_reset()
{
    while(m_stack.size() > 1)
    {
        m_stack.pop();
    }
    C4_ASSERT(m_stack.size() == 1);
    m_state = &m_stack.top();
    m_state->reset(m_file.str, m_root_id);
    m_deindent = nullptr;
}

//-----------------------------------------------------------------------------
bool Parser::_finished_file() const
{
    bool ret = m_state->pos.offset >= m_buf.len;
    if(ret)
    {
        _c4dbgp("finished file!!!");
    }
    return ret;
}

//-----------------------------------------------------------------------------
bool Parser::_finished_line() const
{
    bool ret = ! m_state->line_contents.rem;
    return ret;
}

//-----------------------------------------------------------------------------
void Parser::parse(cspan const& file, cspan const& buf, Node *root)
{
    m_file = file;
    m_buf = buf;
    m_root_id = root->id();
    m_tree = root->tree();
    _reset();

    while( ! _finished_file())
    {
        _scan_line();
        while( ! _finished_line())
        {
            _handle_line();
        }
        _next_line();
    }

    _handle_finished_file();
}

//-----------------------------------------------------------------------------
void Parser::_handle_finished_file()
{
    if(m_stack.empty()) return;

    _c4dbgp("emptying stack...");
    while(m_stack.size() > 1)
    {
        _pop_level();
    }
    m_state = &m_stack.top();
}

//-----------------------------------------------------------------------------
void Parser::_handle_line()
{
    cspan rem = m_state->line_contents.rem;

    _c4dbgs("\n-----------");
    _c4dbgf("handling line %zd", m_state->pos.line);

    C4_ASSERT( ! rem.empty());

    if(m_state->has_all(INDOK))
    {
        if(int jump = m_state->indentation_jump)
        {
            _c4dbgp("indentation jump: %d", jump);
            if(jump < 0)
            {
                _handle_indentation();
            }
        }
    }

    if(_handle_scalar(rem))
    {
        return;
    }

    if(m_state->has_any(RSEQ))
    {
        if(_handle_seq(rem))
        {
            return;
        }
    }
    else if(m_state->has_any(RMAP))
    {
        if(_handle_map(rem))
        {
            return;
        }
    }
    else if(m_state->has_any(RUNK))
    {
        if(_handle_unk(rem))
        {
            return;
        }
    }

    if(_handle_top(rem))
    {
        return;
    }
}

//-----------------------------------------------------------------------------
bool Parser::_handle_scalar(cspan rem)
{
    if(m_state->has_none(RKEY|RVAL|RUNK|RTOP)) return false;

    const bool alnum = isalnum(rem[0]);
    const bool dquot = rem.begins_with('"');
    const bool squot = rem.begins_with('\'');
    const bool block = rem.begins_with('|') || rem.begins_with('>');

    if(alnum || dquot || squot || block)
    {
        _c4dbgp("it's a scalar");
        cspan s = _scan_scalar();
        C4_ASSERT( ! s.empty());

        if(m_state->has_all(RSEQ))
        {
            _append_val(s);
        }
        else if(m_state->has_all(RKEY|RMAP))
        {
            C4_ASSERT(m_state->has_none(RVAL));
            _store_scalar(s);
            _toggle_key_val();
        }
        else if(m_state->has_all(RVAL|RMAP))
        {
            C4_ASSERT(m_state->has_none(RKEY));
            _append_key_val(s); // toggles key val
        }
        else if(m_state->has_all(RUNK))
        {
            C4_ASSERT(m_state->has_none(RSEQ|RMAP));
            // we still don't know whether it's a seq or a map
            // so just store the scalar
            _store_scalar(s);
        }
        else
        {
            _c4err("wtf?");
        }
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_unk(cspan rem)
{
    _c4dbgp("handle_unk");
    const bool start_as_child = (m_state->level != 0) || node(m_state) == nullptr;
    if(rem.begins_with(' '))
    {
        if(m_state->has_all(INDOK))
        {
            _c4dbgp("indentation jump=%d level=%zd #spaces=%d", m_state->indentation_jump, m_state->level, m_state->line_contents.indentation);
            C4_ASSERT(m_state->indentation_jump > 0);
        }
        m_state->line_progressed(m_state->line_contents.indentation);
        return true;
    }
    else if(rem.begins_with("- "))
    {
        _c4dbgp("it's a seq (as_child=%d)", start_as_child);
        _push_level();
        _start_seq(start_as_child);
        m_state->line_progressed(2);
        return true;
    }
    else if(rem == '-')
    {
        _c4dbgp("it's a seq (as_child=%d)", start_as_child);
        _push_level();
        _start_seq(start_as_child);
        m_state->line_progressed(1);
        return true;
    }
    else if(rem.begins_with('['))
    {
        _c4dbgp("it's a seq (as_child=%d)", start_as_child);
        _push_level(/*explicit flow*/true);
        _start_seq(start_as_child);
        m_state->flags |= EXPL;
        m_state->line_progressed(1);
        return true;
    }

    else if(rem.begins_with('{'))
    {
        _c4dbgp("it's a map (as_child=%d)", start_as_child);
        _push_level(/*explicit flow*/true);
        _start_map(start_as_child);
        m_state->flags |= EXPL|RKEY;
        m_state->flags &= ~RVAL;
        m_state->line_progressed(1);
        return true;
    }
    else if(rem.begins_with("? "))
    {
        _c4dbgp("it's a map (as_child=%d)", start_as_child);
        _push_level();
        _start_map(start_as_child);
        m_state->line_progressed(2);
        return true;
    }

    else if(m_state->has_all(SSCL))
    {
        _c4dbgp("there's a stored scalar");
        if(rem.begins_with(", "))
        {
            _c4dbgp("it's a seq (as_child=%d)", start_as_child);
            _start_seq(start_as_child);
            _append_val(_consume_scalar());
            m_state->line_progressed(2);
        }
        else if(rem.begins_with(": "))
        {
            _c4dbgp("it's a map (as_child=%d)", start_as_child);
            _start_map(start_as_child);
            // wait for the val scalar to append the key-val pair
            m_state->line_progressed(2);
            if(rem == ": ")
            {
                _start_unk();
            }
        }
        else if(rem == ":")
        {
            _c4dbgp("it's a map (as_child=%d)", start_as_child);
            _start_map(start_as_child);
            // wait for the val scalar to append the key-val pair
            m_state->line_progressed(1);
            _start_unk();
        }
        else
        {
            _c4err("parse error");
        }
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_seq(cspan rem)
{
    _c4dbgp("handle_seq");
    /*if(m_state->has_any(RNXT))
    {
        _c4err("not implemented");
    }
    else */if(m_state->has_any(RVAL))
    {
        if(rem.begins_with(' '))
        {
            _c4dbgp("starts with spaces");
            if(m_state->has_all(INDOK))
            {
                _c4dbgp("indentation jump=%d level=%zd #spaces=%d", m_state->indentation_jump, m_state->level, m_state->line_contents.indentation);
                m_state->line_progressed(m_state->line_contents.indentation);
                C4_ASSERT(m_state->indentation_jump > 0);
            }
            else
            {
                rem = rem.left_of(rem.first_not_of(' '));
                _c4dbgp("skip %zd spaces", rem.len);
                m_state->line_progressed(rem.len);
            }
            return true;
        }
        else if(rem.begins_with("- "))
        {
            _c4dbgp("expect another val");
            m_state->line_progressed(2);
            return true;
        }
        else if(rem == '-')
        {
            _c4dbgp("start unknown");
            _start_unk();
            m_state->line_progressed(1);
            return true;
        }
        else if(rem.begins_with(", "))
        {
            _c4dbgp("expect another val");
            m_state->flags |= RNXT;
            m_state->line_progressed(2);
            return true;
        }
        else if(rem.begins_with(','))
        {
            _c4dbgp("expect another val");
            m_state->flags |= RNXT;
            m_state->line_progressed(1);
            return true;
        }
        else if(rem.begins_with(']'))
        {
            C4_ASSERT(m_state->has_all(EXPL));
            _c4dbgp("end the sequence");
            m_state->line_progressed(1);
            _pop_level();
            return true;
        }
        else if(rem.begins_with('#'))
        {
            m_state->line_progressed(rem.len);
            return true;
        }
        else if(rem.begins_with('['))
        {
            _c4dbgp("it's a seq");
            _push_level(/*explicit flow*/true);
            _start_seq();
            m_state->flags |= EXPL;
            m_state->line_progressed(1);
            return true;
        }
        else if(rem.begins_with('{'))
        {
            _c4dbgp("it's a map");
            _push_level(/*explicit flow*/true);
            _start_map();
            m_state->flags |= EXPL|RKEY;
            m_state->flags &= ~RVAL;
            m_state->line_progressed(1);
            return true;
        }
        else if(_handle_anchors_and_refs(rem))
        {
            _c4err("not implemented");
            return true;
        }
        else if(_handle_types(rem))
        {
            _c4err("not implemented");
            return true;
        }
        else if(rem.begins_with("---") || rem.begins_with("..."))
        {
            C4_ASSERT(m_state->line_contents.indentation == 0);
            _c4dbgp("end the sequence");
            m_state->line_progressed(3);
            _pop_level();
            return false; // these need further handling
        }
        else
        {
            _c4err("parse error");
        }
    }
    else
    {
        _c4err("internal error");
    }
    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_map(cspan rem)
{
    _c4dbgp("handle_map");
    if(m_state->has_any(RVAL))
    {
        C4_ASSERT(m_state->has_all(SSCL));
        if(rem.begins_with(": "))
        {
            _c4dbgp("wait for val");
            m_state->line_progressed(2);
            return true;
        }
        else if(rem == ':')
        {
            _c4dbgp("start unknown");
            _start_unk();
            m_state->line_progressed(1);
            return true;
        }
        else if(rem.begins_with("- "))
        {
            _c4dbgp("start a seq");
            _push_level();
            _start_seq();
            m_state->line_progressed(2);
            return true;
        }
        else if(rem.begins_with('['))
        {
            _c4dbgp("start a seq");
            _push_level(/*explicit flow*/true);
            _move_scalar_from_top();
            _start_seq();
            m_state->line_progressed(1);
            return true;
        }
        else if(rem.begins_with('{'))
        {
            _c4dbgp("start a map");
            _push_level(/*explicit flow*/true);
            _move_scalar_from_top();
            _start_map();
            m_state->line_progressed(1);
            _toggle_key_val();
            return true;
        }
        else if(rem.begins_with(", "))
        {
            _c4dbgp("expect another key-val");
            m_state->line_progressed(2);
            return true;
        }
        else if(rem.begins_with('}'))
        {
            _c4dbgp("end the map");
            m_state->line_progressed(1);
            _pop_level();
            return true;
        }
        else if(_handle_anchors_and_refs(rem))
        {
            _c4err("not implemented");
            return true;
        }
        else if(_handle_types(rem))
        {
            _c4err("not implemented");
            return true;
        }
        else
        {
            _c4err("parse error");
        }
    }
    else if(m_state->has_any(RKEY))
    {
        if(rem.begins_with(' '))
        {
            rem = rem.left_of(rem.first_not_of(' '));
            m_state->line_progressed(rem.len);
            return true;
        }
        else if(rem.begins_with('#'))
        {
            m_state->line_progressed(rem.len);
            return true;
        }
        else if(m_state->has_any(EXPL))
        {
            if(rem.begins_with(", "))
            {
                m_state->line_progressed(2);
                return true;
            }
            else if(rem.begins_with(','))
            {
                m_state->line_progressed(1);
                return true;
            }
            else if(rem.begins_with('}'))
            {
                _c4dbgp("end the map");
                m_state->line_progressed(1);
                _pop_level();
                return true;
            }
            else
            {
                _c4err("never reach this");
            }
        }
        else
        {
            _c4err("never reach this");
        }
    }
    else
    {
        _c4err("wtf?");
    }
    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_top(cspan rem)
{
    _c4dbgp("handle_top");

    // use the full line, as the following tokens can appear only at top level
    C4_ASSERT(rem == m_state->line_contents.stripped);
    rem = m_state->line_contents.stripped;

    if(rem.begins_with('#'))
    {
        _c4dbgp("a comment line");
        _scan_comment();
        return true;
    }
    else if(rem.begins_with('%'))
    {
        _c4dbgp("%% directive!");
        if(rem.begins_with("%YAML"))
        {
            _c4err("not implemented");
        }
        else if(rem.begins_with("%TAG"))
        {
            _c4err("not implemented");
        }
        else
        {
            _c4err("unknown directive starting with %");
        }
        return true;
    }
    else if(rem.begins_with("---"))
    {
        C4_ASSERT(m_state->level == 0 || m_state->level == 1);
        if(m_state->level == 1)
        {
            // end/start a document
            C4_ASSERT(node(m_state)->is_doc());
            _c4dbgp("end current document");
            _pop_level();
        }

        // start a document
        _c4dbgp("start a document");
        _push_level();
        _start_doc();
        m_state->line_progressed(3);

        // skip spaces after the tag
        rem = rem.subspan(3);
        if(rem.begins_with(' '))
        {
            cspan s = rem.left_of(rem.first_not_of(' '));
            m_state->line_progressed(rem.len);
        }
        return true;
    }
    else if(rem.begins_with("..."))
    {
        _c4dbgp("end current document");
        if( ! m_stack.empty())
        {
            _pop_level();
        }
        m_state->line_progressed(3);
        return true;
    }
    else
    {
        _c4err("parse error");
    }

    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_anchors_and_refs(cspan rem)
{
    if(rem.begins_with('&'))
    {
        _c4err("not implemented");
        return true;
    }
    else if(rem.begins_with('*'))
    {
        _c4err("not implemented");
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
bool Parser::_handle_types(cspan rem)
{
    if(rem.begins_with("!!"))
    {
        _c4err("not implemented");
        return true;
    }
    else if(rem.begins_with('!'))
    {
        _c4err("not implemented");
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
cspan Parser::_scan_scalar()
{
    cspan s = m_state->line_contents.rem;
    if(s.len == 0) return s;

    if(s.begins_with('\''))
    {
        s = _scan_quoted_scalar(/*single*/true);
        return s;
    }
    else if(s.begins_with('"'))
    {
        s = _scan_quoted_scalar(/*single*/false);
        return s;
    }
    else if(s.begins_with('|') || s.begins_with('>'))
    {
        s = _scan_block();
        return s;
    }
    else if(m_state->has_any(RSEQ))
    {
        if(m_state->has_all(RKEY))
        {
            _c4err("internal error");
        }
        else if(m_state->has_all(RVAL))
        {
            _c4dbgp("RSEQ|RVAL");
            s = s.left_of(s.first_of(",]#"));
            s = s.trimr(' ');
        }
        else
        {
            _c4err("parse error");
        }
    }
    else if(m_state->has_any(RMAP))
    {
        size_t colon_space = s.find(": ");
        if(colon_space == npos)
        {
            colon_space = s.find(":");
            C4_ASSERT(s.len > 0);
            if(colon_space != s.len-1)
            {
                colon_space = npos;
            }
        }

        if(m_state->has_all(RKEY))
        {
            _c4dbgp("RMAP|RKEY");
            if(m_state->has_any(CPLX))
            {
                C4_ASSERT(m_state->has_any(RMAP));
                s = s.left_of(colon_space);
            }
            else
            {
                s = s.triml(' ');
                s = s.left_of(colon_space);
                s = s.trimr(' ');
            }
        }
        else if(m_state->has_all(RVAL))
        {
            _c4dbgp("RMAP|RVAL");
            s = s.left_of(s.first_of(",}#"));
            s = s.trim(' ');
        }
        else
        {
            _c4err("parse error");
        }
    }
    else if(m_state->has_all(RUNK))
    {
        s = s.left_of(s.first_of(",: #"));
    }
    else
    {
        _c4err("not implemented");
    }

    _c4dbgp("scalar was '%.*s'", _c4prsp(s));

    m_state->line_progressed(s.str - m_state->line_contents.rem.str + s.len);

    return s;
}

//-----------------------------------------------------------------------------
void Parser::_scan_line()
{
    if(m_state->pos.offset >= m_buf.len) return;
    char const* b = &m_buf[m_state->pos.offset];
    char const* e = b;
    // get the line stripped of newline chars
    while(e != m_buf.end() && (*e != '\r' && *e != '\n'))
    {
        ++e;
    }
    size_t len = e - b;
    cspan stripped = m_buf.subspan(m_state->pos.offset, len);
    // advance pos to include the first line ending
    while(e != m_buf.end() && (*e == '\r' || *e == '\n'))
    {
        ++e;
        ++len;
    }
    cspan full = m_buf.subspan(m_state->pos.offset, len);
    m_state->line_scanned(full, stripped);
}

//-----------------------------------------------------------------------------

void Parser::_push_level(bool explicit_flow_chars)
{
    _c4dbgp("pushing level! sz=%zd (+1)", m_stack.size());
    if(node(m_state) == nullptr)
    {
        C4_ASSERT( ! explicit_flow_chars);
        return;
    }
    size_t st = RUNK;
    if(explicit_flow_chars)
    {
        st |= EXPL;
    }
    _c4dbgp("stacking node %zd", node(m_state)->id());
    m_stack.push(*m_state);
    m_state = &m_stack.top();
    m_state->flags = st;
    m_state->node_id = NONE;
    ++m_state->level;
}

void Parser::_pop_level()
{
    _c4dbgp("popping level! sz=%zd (-1)", m_stack.size());
    if(m_state->has_any(RMAP))
    {
        _stop_map();
    }
    else if(m_state->has_any(RSEQ))
    {
        _stop_seq();
    }
    else if(node(m_state)->is_doc())
    {
        _stop_doc();
    }
    else
    {
        _c4err("internal error");
    }
    C4_ASSERT(m_stack.size() > 1);
    _c4dbgp("popping node %zd newtop %zd", node(m_state)->id(), m_stack.empty() ? NONE : node(m_stack.top())->id());
    m_stack.top(1)._prepare_pop(m_stack.top());
    m_stack.pop();
    m_state = &m_stack.top();
    if(m_state->has_any(RMAP))
    {
        _toggle_key_val();
    }
    if(m_state->line_contents.indentation == 0)
    {
        //C4_ASSERT(m_state->has_none(RTOP));
        m_state->flags |= RTOP;
    }
}

//-----------------------------------------------------------------------------
void Parser::_start_unk(bool as_child)
{
    _c4dbgp("start_unk");
    _push_level();
    _move_scalar_from_top();
}

//-----------------------------------------------------------------------------
void Parser::_start_doc(bool as_child)
{
    _c4dbgp("start_doc (as child=%d)", as_child);
    m_state->flags |= RUNK;
    C4_ASSERT(node(m_stack.bottom()) == node(m_root_id));
    Node* parent = m_stack.size() < 2 ? node(m_root_id) : node(m_stack.top(1));
    size_t parent_id = parent->id();
    C4_ASSERT(parent != nullptr);
    C4_ASSERT(parent->is_root());
    C4_ASSERT(node(m_state) == nullptr || node(m_state) == node(m_root_id));
    if(as_child)
    {
        if( ! parent->is_stream())
        {
            parent->to_stream();
        }
        m_state->node_id = m_tree->append_child(parent_id);
        node(m_state)->to_doc();
    }
    else
    {
        C4_ASSERT(parent->is_seq() || parent->empty());
        m_state->node_id = parent->id();
        if( ! parent->is_doc())
        {
            parent->to_doc(DOC);
        }
    }
    _c4dbgp("start_doc: id=%zd", node(m_state)->id());
}

void Parser::_stop_doc()
{
    _c4dbgp("stop_doc");
    C4_ASSERT(node(m_state)->is_doc());
}

//-----------------------------------------------------------------------------
void Parser::_start_map(bool as_child)
{
    _c4dbgp("start_map (as child=%d)", as_child);
    m_state->flags |= RMAP|RVAL;
    m_state->flags &= ~RKEY;
    m_state->flags &= ~RUNK;
    C4_ASSERT(node(m_stack.bottom()) == node(m_root_id));
    Node* parent = m_stack.size() < 2 ? node(m_root_id) : node(m_stack.top(1));
    C4_ASSERT(parent != nullptr);
    C4_ASSERT(node(m_state) == nullptr || node(m_state) == node(m_root_id));
    if(as_child)
    {
        m_state->node_id = m_tree->append_child(parent->id());
        if(m_state->has_all(SSCL))
        {
            C4_ASSERT(parent->is_map());
            cspan name = _consume_scalar();
            node(m_state)->to_map(name);
            _c4dbgp("start_map: id=%zd name='%.*s'", node(m_state)->id(), _c4prsp(node(m_state)->key()));
        }
        else
        {
            node(m_state)->to_map();
            _c4dbgp("start_map: id=%zd", node(m_state)->id());
        }
    }
    else
    {
        C4_ASSERT(parent->is_map() || parent->empty());
        m_state->node_id = parent->id();
        parent->to_map();
        _move_scalar_from_top();
        _c4dbgp("start_map: id=%zd", node(m_state)->id());
    }
}

void Parser::_stop_map()
{
    _c4dbgp("stop_map");
    C4_ASSERT(node(m_state)->is_map());
}

//-----------------------------------------------------------------------------
void Parser::_start_seq(bool as_child)
{
    _c4dbgp("start_seq (as child=%d)", as_child);
    m_state->flags |= RSEQ|RVAL;
    m_state->flags &= ~RUNK;
    C4_ASSERT(node(m_stack.bottom()) == node(m_root_id));
    Node* parent = m_stack.size() < 2 ? node(m_root_id) : node(m_stack.top(1));
    C4_ASSERT(parent != nullptr);
    C4_ASSERT(node(m_state) == nullptr || node(m_state) == node(m_root_id));
    if(as_child)
    {
        m_state->node_id = m_tree->append_child(parent->id());
        if(m_state->has_all(SSCL))
        {
            cspan name = _consume_scalar();
            node(m_state)->to_seq(name);
            _c4dbgp("start_seq: id=%zd name='%.*s'", node(m_state)->id(), _c4prsp(node(m_state)->key()));
        }
        else
        {
            node(m_state)->to_seq();
            _c4dbgp("start_seq: id=%zd", node(m_state)->id());
        }
    }
    else
    {
        C4_ASSERT(parent->is_seq() || parent->empty());
        m_state->node_id = parent->id();
        parent->to_seq(SEQ);
        _move_scalar_from_top();
        _c4dbgp("start_seq: id=%zd", node(m_state)->id());
    }
}

void Parser::_stop_seq()
{
    _c4dbgp("stop_seq");
    C4_ASSERT(node(m_state)->is_seq());
}

//-----------------------------------------------------------------------------
void Parser::_append_val(cspan const& val)
{
    C4_ASSERT( ! m_state->has_all(SSCL));
    C4_ASSERT(node(m_state) != nullptr);
    C4_ASSERT(node(m_state)->is_seq());
    _c4dbgp("append val: '%.*s'", _c4prsp(val));
    size_t ch = m_tree->append_child(m_state->node_id);
    m_tree->get(ch)->to_val(val);
    _c4dbgp("append val: id=%zd name='%.*s' val='%.*s'", node(m_state)->last_child()->id(), _c4prsp(node(m_state)->last_child()->m_key), _c4prsp(node(m_state)->last_child()->m_val));
}

void Parser::_append_key_val(cspan const& val)
{
    C4_ASSERT(node(m_state)->is_map());
    cspan key = _consume_scalar();;
    _c4dbgp("append key-val: '%.*s' '%.*s'", _c4prsp(key), _c4prsp(val));
    size_t ch = m_tree->append_child(m_state->node_id);
    m_tree->get(ch)->to_keyval(key, val);
    _c4dbgp("append key-val: id=%zd name='%.*s' val='%.*s'", node(m_state)->last_child()->id(), _c4prsp(node(m_state)->last_child()->key()), _c4prsp(node(m_state)->last_child()->val()));
    _toggle_key_val();
}

void Parser::_toggle_key_val()
{
    if(m_state->flags & RKEY)
    {
        m_state->flags &= ~RKEY;
        m_state->flags |= RVAL;
    }
    else
    {
        m_state->flags |= RKEY;
        m_state->flags &= ~RVAL;
    }
}

//-----------------------------------------------------------------------------
void Parser::_store_scalar(cspan const& s)
{
    _c4dbgp("storing scalar: '%.*s'", _c4prsp(s));
    C4_ASSERT(m_state->has_none(SSCL));
    m_state->flags |= SSCL;
    m_state->scalar = s;
}

cspan Parser::_consume_scalar()
{
    _c4dbgp("consuming scalar: '%.*s' (flag: %d))", _c4prsp(m_state->scalar), m_state->scalar & SSCL);
    C4_ASSERT(m_state->flags & SSCL);
    cspan s = m_state->scalar;
    m_state->flags &= ~SSCL;
    m_state->scalar.clear();
    return s;
}

void Parser::_move_scalar_from_top()
{
    if(m_stack.size() < 2) return;
    State &prev = m_stack.top(1);
    C4_ASSERT(m_state == &m_stack.top());
    C4_ASSERT(m_state != &prev);
    if(prev.flags & SSCL)
    {
        _c4dbgp("moving scalar: '%.*s'@%zd (overwriting '%.*s'@%zd)", _c4prsp(prev.scalar), &prev-m_stack.begin(), _c4prsp(m_state->scalar), m_state-m_stack.begin());
        m_state->flags |= (prev.flags & SSCL);
        m_state->scalar = prev.scalar;
        prev.flags &= ~SSCL;
        prev.scalar.clear();
    }
}

//-----------------------------------------------------------------------------
int Parser::_handle_indentation()
{
    int jump = m_state->indentation_jump;
    if(jump < 0)
    {
        _c4dbgp("indentation decreased %d space%s!", (-jump), (-jump)>1?"s":"");
        for(size_t i = 0; i < jump; ++i)
        {
            _pop_level();
        }
    }
    else if(jump > 0)
    {
        _c4err("never reach this");
        // level must be pushed elsewhere;
        // too complicated to deal with here
    }
    return jump;
}

//-----------------------------------------------------------------------------
cspan Parser::_scan_comment()
{
    cspan s = m_state->line_contents.rem;
    C4_ASSERT(s.begins_with('#'));
    m_state->line_progressed(s.len);
    s = s.subspan(1);
    s = s.right_of(s.first_not_of(' '), /*include_pos*/true);
    return s;
}

//-----------------------------------------------------------------------------
cspan Parser::_scan_quoted_scalar(const char q)
{
    // quoted scalars can spread over multiple lines!
    // nice explanation here: http://yaml-multiline.info/

    cspan s = m_state->line_contents.rem;
    C4_ASSERT(q == '\'' || q == '\"');

    // find the pos of the matching quote
    size_t pos = npos, sum = 0;
    C4_ASSERT(s.begins_with(q));

    // skip the opening quote
    m_state->line_progressed(1);

    bool needs_filter = true;

    while( ! _finished_file())
    {
        _scan_line();
        s = m_state->line_contents.rem;

        for(size_t i = 1; i < s.len; ++i)
        {
            const char prev = s.str[i-1];
            const char curr = s.str[i];
            if(q == '\'') // scalars with single quotes
            {
                // ... are only escaped with two single quotes
                if(curr == '\'')
                {
                    if(prev != '\'')
                    {
                        pos = i;
                        break;
                    }
                    else
                    {
                        // there are two single quotes, so needs filter
                        needs_filter = true;
                    }
                }
            }
            else // scalars with double quotes
            {
                // every \ is an escape
                if(curr == '"')
                {
                    if(prev != '\\')
                    {
                        pos = i;
                        break;
                    }
                    else
                    {
                        needs_filter = true;
                    }
                }
                else if(curr == '\\')
                {
                    needs_filter = true;
                }
            }
        }

        if(pos != npos)
        {
            m_state->line_progressed(pos);
            pos += sum;
        }
        else
        {
            m_state->line_progressed(s.len);
            sum += s.len;
            break;
        }

        _next_line();
    }
    if(pos == npos)
    {
        _c4err("reached end of file while looking for closing quote");
    }
    C4_ASSERT(s[pos] == q);
    s = s.subspan(1, pos - 1);

    if(needs_filter)
    {
        cspan ret = _filter_quoted_scalar(s, q);
        C4_ASSERT(ret.len < s.len);
        return ret;
    }

    return s;
}

//-----------------------------------------------------------------------------
cspan Parser::_scan_block()
{
    // nice explanation here: http://yaml-multiline.info/
    cspan s = m_state->line_contents.rem;
    C4_ASSERT(s.begins_with('|') || s.begins_with('>'));

    // parse the spec
    BlockStyle_e newline = s.begins_with('>') ? BLOCK_FOLD : BLOCK_LITERAL;
    BlockChomp_e chomp = CHOMP_CLIP; // default to clip unless + or - are used
    size_t indentation = npos; // have to find out if no spec is given
    if(s.len > 1)
    {
        s = s.subspan(1);
        if(s[0] == '-')
        {
            chomp = CHOMP_STRIP;
            s = s.subspan(1);
        }
        else if(s[0] == '+')
        {
            chomp = CHOMP_KEEP;
            s = s.subspan(1);
        }

        // from here to the end, only digits are considered
        cspan digits = s.left_of(s.first_not_of("0123456789"));
        if(digits)
        {
            if( ! _read_decimal(digits, &indentation))
            {
                _c4err("parse error: could not read decimal");
            }
        }
    }

    // finish the current line
    _next_line();

    // start with a zero-length block, already pointing at the right place
    cspan raw_block = m_state->line_contents.full.subspan(0, 0);
    C4_ASSERT(raw_block.begin() == m_state->line_contents.full.begin());

    // read every full line into a raw block,
    // from which newlines are to be stripped as needed
    size_t num_lines = 0, first = m_state->pos.line;
    while( ! _finished_file() && m_state->line_contents.indentation >= indentation)
    {
        _scan_line();
        raw_block.len += m_state->line_contents.full.len;
        _next_line();
        ++num_lines;
    }
    C4_ASSERT(m_state->pos.line == (first + num_lines));
    (void)num_lines; // prevent warning
    (void)first;

    // ok! now we strip the newlines and spaces according to the specs
    s = _filter_raw_block(raw_block, newline, chomp, indentation);

    return s;
}

//-----------------------------------------------------------------------------
cspan Parser::_filter_quoted_scalar(cspan const& s, const char q)
{
    _c4err("not implemented");
    return {};
}

//-----------------------------------------------------------------------------
cspan Parser::_filter_raw_block(cspan const& block, BlockStyle_e style, BlockChomp_e chomp, size_t indentation)
{
    C4_ASSERT(block.ends_with('\n') || block.ends_with('\r'));

    // with 0-indentation literal blocks we don't need
    // to erase characters from the middle of the string,
    // so we can just reuse the block and chomp at the end
    // by getting a subspan of the block
    if(style == BLOCK_LITERAL && indentation == 0)
    {
        switch(chomp)
        {
        case CHOMP_KEEP:
            return block;
        case CHOMP_STRIP: // strip everything
            {
                auto pos = block.last_not_of("\r\n");
                C4_ASSERT(pos != npos);
                auto ret = block.left_of(pos, /*include_pos*/true);
                return ret;
            }
        case CHOMP_CLIP: // keep a single newline
            {
                auto pos = block.last_not_of("\r\n");
                C4_ASSERT(pos != npos && pos+1 < block.len);
                ++pos;
                // deal for \r\n sequences
                if(block[pos] == '\r')
                {
                    C4_ASSERT(pos+1 < block.len);
                    ++pos;
                    C4_ASSERT(block[pos] == '\n');
                }
                auto ret = block.left_of(pos, /*include_pos*/true);
                return ret;
            }
        default:
            _c4err("unknown style");
        }
    }

    _c4err("not implemented");
    return {};
}

//-----------------------------------------------------------------------------
bool Parser::_read_decimal(cspan const& str, size_t *decimal)
{
    C4_ASSERT(str.len > 1);
    size_t n = 0, c = 0;
    for(size_t i = 0; i < str.len; ++i)
    {
        if(str.str[i] < '0' || str.str[i] > '9') return false;
        n = n*10 + c;
    }
    *decimal = n;
    return true;
}

//-----------------------------------------------------------------------------
size_t Parser::_count_nlines(cspan src)
{
    size_t n = (src.len > 0);
    while(src.len > 0)
    {
        n += (src.begins_with('\n') || src.begins_with('\r'));
        src = src.subspan(1);
    }
    return n;
}

//-----------------------------------------------------------------------------
void Parser::_err(const char *fmt, ...) const
{
    char errmsg[RYML_ERRMSG_SIZE];
    int len = sizeof(errmsg);

    va_list args;
    va_start(args, fmt);
    len = _fmt_msg(errmsg, len, fmt, args);
    va_end(args);
    RymlCallbacks::error(errmsg, len);
}

#ifdef RYML_DBG
//-----------------------------------------------------------------------------
void Parser::_dbg(const char *fmt, ...) const
{
    char errmsg[RYML_ERRMSG_SIZE];
    int len = sizeof(errmsg);

    va_list args;
    va_start(args, fmt);
    len = _fmt_msg(errmsg, len, fmt, args);
    va_end(args);
    printf("%.*s", len, errmsg);
}
#endif

//-----------------------------------------------------------------------------
int Parser::_fmt_msg(char *buf, int buflen, const char *fmt, va_list args) const
{
    int len = buflen;
    int pos = 0;
    auto const& lc = m_state->line_contents;

#define _wrapbuf() pos += del; len -= del; if(len < 0) { pos = 0; len = buflen; }

    // first line: print the message
    int del = vsnprintf(buf + pos, len, fmt, args);
    _wrapbuf();
    del = snprintf(buf + pos, len, "\n");
    _wrapbuf();

    // next line: print the yaml src line
    if(m_file)
    {
        del = snprintf(buf + pos, len, "%.*s:%d: '", (int)m_file.len, m_file.str, m_state->pos.line);
    }
    else
    {
        del = snprintf(buf + pos, len, "line %zd: '", m_state->pos.line, m_state->pos.line);
    }
    int offs = del;
    _wrapbuf();
    del = snprintf(buf + pos, len, "%.*s' (sz=%zd)\n",
                   (int)lc.stripped.len, lc.stripped.str, lc.stripped.len);
    _wrapbuf();

    // next line: highlight the remaining portion of the previous line
    if(lc.rem.len)
    {
        size_t firstcol = lc.rem.begin() - lc.full.begin();
        size_t lastcol = firstcol + lc.rem.len;
        del = snprintf(buf + pos, len, "%*s", offs+firstcol, ""); // this works only for spaces....
        _wrapbuf();
        // the %*s technique works only for spaces, so put the characters directly
        del = (int)lc.rem.len;
        for(int i = 0; i < del && i < len; ++i) { buf[pos + i] = (i ? '~' : '^'); }
        _wrapbuf();
        del = snprintf(buf + pos, len, "  (cols %zd-%zd)\n", firstcol+1, lastcol+1);
        _wrapbuf();
    }
    else
    {
        del = snprintf(buf + pos, len, "\n");
        _wrapbuf();
    }

    // next line: print the state flags
    {
        del = snprintf(buf + pos, len, "top state: ");
        _wrapbuf();

        bool gotone = false;
#define _prflag(fl)                                                     \
        if((m_state->flags & (fl)) == (fl))                             \
        {                                                               \
            if(!gotone)                                                 \
            {                                                           \
                gotone = true;                                          \
            }                                                           \
            else                                                        \
            {                                                           \
                del = snprintf(buf + pos, len, "|");                    \
                _wrapbuf();                                             \
            }                                                           \
            del = snprintf(buf + pos, len, #fl);                        \
            _wrapbuf();                                                 \
        }

        _prflag(RTOP);
        _prflag(RUNK);
        _prflag(RMAP);
        _prflag(RSEQ);
        _prflag(EXPL);
        _prflag(CPLX);
        _prflag(RKEY);
        _prflag(RVAL);
        _prflag(RNXT);
        _prflag(SSCL);
#undef _prflag

        del = snprintf(buf + pos, len, "\n");
        _wrapbuf();
    }

#undef _wrapbuf

    return pos;
}

} // namespace ryml
} // namespace c4
