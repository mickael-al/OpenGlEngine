#ifndef __WFC_CELL__
#define __WFC_CELL__

#include <vector>
#include <unordered_set>

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UPLEFT,
    UPRIGHT,
    DOWNLEFT,
    DOWNRIGHT,
};


class WfcCell
{    
public:
    WfcCell();
    WfcCell(std::vector<int>& all_posibility, int unique_value);

    void update(std::vector<int>& all_posibility);
    std::vector<int> & getPossibilites();
    void computeEntropy();
    void collapse();
    int get_entropy();
    bool is_collapsed();
    int get_value();

private:
    std::vector<int> m_possibilites;
    bool m_collapsed = false;
    int m_value = -1;
    int m_entropy = 0;
};

#endif//!__WFC_CELL__