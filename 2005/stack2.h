

#pragma once

#include <stack>


// --------------------------------------------------------------------------------
template<typename T>
class Stack2
{
public:
  typedef typename std::vector<T>::reverse_iterator iterator;
  typedef typename std::vector<T>::const_reverse_iterator const_iterator;

  void Pop()
  {
    m_container.pop_back();
  }
  void Push(const T& val)
  {
    m_container.push_back(val);
  }
  const T& Top() const
  {
    return m_container.back();
  }
  T& Top()
  {
    return m_container.back();
  }

  // walk from top of stack to the bottom.
  iterator begin()
  {
    return m_container.begin();
  }

  iterator end()
  {
    return m_container.end();
  }

  const_iterator begin() const
  {
    return m_container.rbegin();
  }

  const_iterator end() const
  {
    return m_container.rend();
  }

  size_t size() const
  {
    return m_container.size();
  }

private:
  std::vector<T> m_container;
};

