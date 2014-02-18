// Matrix math:
// http://people.hofstra.edu/faculty/Stefan_Waner/RealWorld/tutorialsf1/unit3_2.html
// * The product AB has as many rows as A and as many columns as B
// * To obtain the i,j entry of AB, multiply Row i of A by Column j of B


#pragma once


#include <vector>


// class representing a two-dimentional matrix
template<typename T>
class C2DMatrix
{
public:
    typedef C2DMatrix<T> This_T;
    typedef T Cell_T;
    typedef long Coord_T;

    C2DMatrix() :
        m_nColumns(0)
    {
    }

    void assign(const This_T& m)
    {
        SetRows(m.GetRows());
        SetColumns(m.GetColumns());
        Cell_T temp;
        Coord_T x, y;
        for(x=0;x<GetColumns();x++)
        {
            for(y=0;y<GetRows();y++)
            {
                m.GetValue(x,y,temp);
                SetValue(x,y,temp);
            }
        }
    }

    void SetRows(const Coord_T& v)
    {
        m_Rows.resize(v);
        SetColumns(m_nColumns);
    }

    void SetColumns(const Coord_T& v)
    {
        // for each column, set the row size.
        Rows_T::iterator it;
        for(it=m_Rows.begin();it!=m_Rows.end();it++)
        {
            it->resize(v);
        }

        m_nColumns = v;
    }

    Coord_T GetRows() const { return (Coord_T)m_Rows.size(); }
    Coord_T GetColumns() const { return m_nColumns; }

    // x and y are 0-based.
    bool SetValue(const Coord_T& x, const Coord_T& y, const T& val)
    {
        // coord validation
        if(x < 0) return false;
        if(y < 0) return false;
        if(x >= GetColumns()) return false;
        if(y >= GetRows()) return false;

        m_Rows[y][x] = val;
        return true;
    }

    bool GetValue(const Coord_T& x, const Coord_T& y, T& val) const
    {
        // coord validation
        if(x < 0) return false;
        if(y < 0) return false;
        if(x >= GetColumns()) return false;
        if(y >= GetRows()) return false;

        val = m_Rows[y][x];
        return true;
    }

    bool Output()
    {
        Column_T::iterator itX;
        Rows_T::iterator itY;
        for(itY=m_Rows.begin();itY!=m_Rows.end();itY++)
        {
            std::cout << "| ";
            for(itX=itY->begin();itX!=itY->end();itX++)
            {
                std::cout << (*itX) << "  ";
            }

            std::cout << "| " << std::endl;
        }

        std::cout << std::endl;
        return true;
    }

    // m must have the same # of rows as this, and only 1 column.
    bool ScalarMultiplyColumns(const This_T& m)
    {
        bool r = false;
        Cell_T temp;

        if(m.GetRows() == GetRows())
        {
            if(m.GetColumns() == 1)
            {
                Coord_T x, y;
                for(x=0;x<GetColumns();x++)
                {
                    for(y=0;y<GetRows();y++)
                    {
                        m.GetValue(0, y, temp);
                        m_Rows[y][x] *= temp;
                    }
                }
            }
        }

        return r;
    }

    bool ScalarMultiplyRows(const This_T& m)
    {
        bool r = false;
        Cell_T temp;

        if(m.GetRows() == 1)
        {
            if(m.GetColumns() == GetColumns())
            {
                Coord_T x, y;
                for(x=0;x<GetColumns();x++)
                {
                    for(y=0;y<GetRows();y++)
                    {
                        m.GetValue(x, 0, temp);
                        m_Rows[y][x] *= temp;
                    }
                }
            }
        }

        return r;
    }

    void ScalarMultiplyUniform(const Cell_T& val)
    {
        Coord_T x, y;
        for(x=0;x<GetColumns();x++)
        {
            for(y=0;y<GetRows();y++)
            {
                m_Rows[y][x] *= val;
            }
        }
    }

private:
    typedef std::vector<Cell_T> Column_T;// column is a vector of cells
    typedef std::vector<Column_T> Rows_T;// a row is many columns

    Coord_T m_nColumns;// number of rows
    Rows_T m_Rows;
};


template<typename T>
bool matrix_add(const C2DMatrix<T>& a, const C2DMatrix<T>& b, C2DMatrix<T>& r)
{
    if(b.GetRows() != a.GetRows()) return false;
    if(b.GetColumns() != a.GetColumns()) return false;

    r.SetRows(b.GetRows());
    r.SetColumns(b.GetColumns());

    C2DMatrix<T>::Coord_T x, y;
    C2DMatrix<T>::Cell_T add_a, add_b, acc;

    for(x=0;x<b.GetColumns();x++)
    {
        for(y=0;y<b.GetRows();y++)
        {
            a.GetValue(x, y, add_a);
            b.GetValue(x, y, add_b);
            acc = a+b;
            r.SetValue(x, y, acc);
        }
    }

    return true;
}


template<typename T>
bool matrix_mul(const C2DMatrix<T>& a, const C2DMatrix<T>& b, C2DMatrix<T>& r)
{
    // Make sure the product is defined.
    if(b.GetRows() != a.GetColumns()) return false;

    r.SetRows(a.GetRows());
    r.SetColumns(b.GetColumns());

    C2DMatrix<T>::Coord_T xa, ya;// row/column of A
    C2DMatrix<T>::Coord_T xb;// row/column of B
    C2DMatrix<T>::Cell_T acc, mul_a, mul_b;// accumulator

    // For each row of A
    for(ya=0;ya<a.GetRows();ya++)
    {
        // for each column of B
        for(xb=0;xb<b.GetColumns();xb++)
        {
            // for each column in A
            acc = 0;
            for(xa=0;xa<a.GetColumns();xa++)
            {
                b.GetValue(xb, xa, mul_a);
                a.GetValue(xa, ya, mul_b);
                acc += mul_a * mul_b;
            }

            r.SetValue(xb, ya, acc);
        }
    }

    return true;
}


template<typename T>
bool matrix_transpose(const C2DMatrix<T>& M, C2DMatrix<T>& N)
{
    T temp;
    N.SetRows(M.GetColumns());
    N.SetColumns(M.GetRows());
    C2DMatrix<T>::Coord_T x, y;
    for(x=0;x<M.GetColumns();x++)
    {
        for(y=0;y<M.GetRows();y++)
        {
            // swap 
            M.GetValue(x, y, temp);
            N.SetValue(y, x, temp);
        }
    }

    return true;
}


template<typename TCell, typename TDet>
bool matrix_determinant(const C2DMatrix<TCell>& m, TDet& det)
{
    C2DMatrix<TCell>::Coord_T k = m.GetColumns();

    // Ensure it's square.
    if(k != m.GetRows()) return false;
    if(k == 0) return false;
    // If a 1x1, just return the 1
    if(k == 1)
    {
        C2DMatrix<TCell>::Cell_T temp;
        m.GetValue(0,0, temp);
        det = (TDet)(temp);
        return true;
    }
    // If a 2x2, solve.
    if(k == 2)
    {
        // In |a b| , the det() is ad-bc
        //    |c d|
        C2DMatrix<TCell>::Cell_T a, b, c, d;
        m.GetValue(0, 0, a);
        m.GetValue(1, 0, b);
        m.GetValue(0, 1, c);
        m.GetValue(1, 1, d);
        det = (TDet)((a*d)-(c*b));
        return true;
    }
    // At least a 3x3 matrix:
    // We will have to recurse down by "minors".
    /*
        | a11 a12 a13 ... a1k |
        | a21 a22 a23 ... a2k |            | a22 a23 ... a2k |             | a21 a23 ... a2k|                 | a21 a22 ... a2(k-1)|
        |  :   :   :       :  | =  ( a11 * |  :   :       :  | ) - ( a12 * |  :   :       : | ) ... ± ( a1k * |  :   :         :   | )
        |  :   :   :       :  |            | ak2 ak3 ... akk |             | ak1 ak3 ... akk|                 | ak1 ak2 ... ak(k-1)|
        | ak1 ak2 ak3 ... akk |            
    */
    // Start with 0.
    det = 0;

    C2DMatrix<TCell>::Coord_T x, x2, y, cx;// major x, major x, minor y, cumulative minor row
    C2DMatrix<TCell>::Cell_T temp;
    C2DMatrix<TCell> Minor;
    TDet sub_det;// a single a11*|...| operation (what's in parenthesis up in the example)
    bool bAdd = true;// switches every iteration

    Minor.SetColumns(k-1);
    Minor.SetRows(k-1);

    for(x=0;x<k;x++)
    {
        // construct the minor matrix
        // rows BEFORE our current
        cx = 0;// current minor row
        for(x2=0;x2<x;x2++)
        {
            for(y=1;y<k;y++)
            {
                m.GetValue(x2, y, temp);
                Minor.SetValue(cx, y-1, temp);
            }
            cx++;
        }
        // rows AFTER our current
        for(x2=x+1;x2<k;x2++)
        {
            for(y=1;y<k;y++)
            {
                m.GetValue(x2, y, temp);
                Minor.SetValue(cx, y-1, temp);
            }
            cx++;
        }

        // calculate the intermediate (sub) determinant
        matrix_determinant(Minor, sub_det);

        m.GetValue(x, 0, temp);

        // Accumulate.
        if(bAdd)
        {
            det += temp*sub_det;
        }
        else
        {
            det -= temp*sub_det;
        }
        bAdd = !bAdd;
    }

    return true;
}


template<typename T>
bool matrix_invert(const C2DMatrix<T>& m, C2DMatrix<T>& n)
{
    // Only works for a 3x3 matrix
    bool r = false;
    if(m.GetColumns() == 3)
    {
        if(m.GetRows() == 3)
        {
            C2DMatrix<T>::Cell_T det;
            if(matrix_determinant<T, T>(m, det))
            {
                if(det)
                {
                    T a, b, c, d, e, f, g, h, i;
                    m.GetValue(0, 0, a);
                    m.GetValue(1, 0, b);
                    m.GetValue(2, 0, c);
                    m.GetValue(0, 1, d);
                    m.GetValue(1, 1, e);
                    m.GetValue(2, 1, f);
                    m.GetValue(0, 2, g);
                    m.GetValue(1, 2, h);
                    m.GetValue(2, 2, i);

                    n.SetColumns(3);
                    n.SetRows(3);
                    n.SetValue(0, 0, (e*i-h*f)/det);
                    n.SetValue(1, 0, -(b*i-h*c)/det);
                    n.SetValue(2, 0, (b*f-e*c)/det);
                    n.SetValue(0, 1, -(d*i-g*f)/det);
                    n.SetValue(1, 1, (a*i-g*c)/det);
                    n.SetValue(2, 1, -(a*f-d*c)/det);
                    n.SetValue(0, 2, (d*h-g*e)/det);
                    n.SetValue(1, 2, -(a*h-g*b)/det);
                    n.SetValue(2, 2, (a*e-d*b)/det);

                    r = true;
//    |a b c|
//M = |d e f|
//    |g h i|
//
// -1       1    | (ei-hf) -(bi-hc)  (bf-ec)|
//M    =  ------ |-(di-gf)  (ai-gc) -(af-dc)|
//        det(M) | (dh-ge) -(ah-gb)  (ae-db)|

                }
            }
        }
    }
    return r;
}



