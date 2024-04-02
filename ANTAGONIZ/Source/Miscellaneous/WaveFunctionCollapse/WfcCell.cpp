#include "WfcCell.hpp"
#include<algorithm>
#include<iterator>
#include <iostream>

WfcCell::WfcCell()
{

}

WfcCell::WfcCell(std::vector<int> & all_posibility,int unique_value)
{
    std::copy(all_posibility.begin(), all_posibility.end(), back_inserter(m_possibilites));
    m_entropy = unique_value;
}

std::vector<int> & WfcCell::getPossibilites()
{
    return m_possibilites;
}

void WfcCell::computeEntropy()
{
    int count_entropy = 0;
    std::unordered_set<int> uniqueElements;
    for (int i : m_possibilites)
    {
        if (uniqueElements.insert(i).second)
        {
            count_entropy++;
        }
    }
    m_entropy = count_entropy;
}

void WfcCell::update(std::vector<int>& nposibility)
{
    if (this->m_collapsed)
    {
        return;
    }
    std::sort(m_possibilites.begin(), m_possibilites.end());
    std::sort(nposibility.begin(), nposibility.end());

   /* std::cout << "m_possibilites : ";
    for (const auto& elem : m_possibilites)
    {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    std::cout << "nposibility    : ";
    for (const auto& elem : nposibility)
    {
        std::cout << elem << " ";
    }
    std::cout << std::endl;*/

    std::vector<int> nv;
    std::set_intersection(m_possibilites.begin(), m_possibilites.end(),
        nposibility.begin(), nposibility.end(),
        std::back_inserter(nv));

    int count_entropy = 0;
    std::unordered_set<int> uniqueElements;
    for (int i : nv)
    {
        if (uniqueElements.insert(i).second)
        {
            count_entropy++;
        }
    }
    m_entropy = count_entropy;

    if (nv.size() > 0)
    {
        m_possibilites = nv;
    }

    /*std::cout << "nv             : ";
    for (const auto& elem : nv)
    {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;*/
}

void WfcCell::collapse()
{
    if (this->m_possibilites.size() >= 1)
    {
        this->m_value = this->m_possibilites.at(std::rand() % this->m_possibilites.size());
    }
    else
    {        
        this->m_value = -2;        
    }
    m_entropy = 0;
    this->m_possibilites.clear();    
    this->m_possibilites.push_back(this->m_value);
    this->m_collapsed = true;

    return;
}

int WfcCell::get_entropy()
{
    return m_entropy;
}

int WfcCell::get_value()
{
    return this->m_value;
}

bool WfcCell::is_collapsed()
{
	return this->m_collapsed;
}
