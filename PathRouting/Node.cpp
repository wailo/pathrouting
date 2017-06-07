#include "Node.h"
#include <limits>
#include "QuadTree.h"
// Initiaize static member
unsigned int Node::nodecount = 0;


Node::Node(QuadTree* p_parentTree):
    m_parentTree(p_parentTree),
    m_draw( true )
{
  // Increment the node counter
  nodecount++;

  // Set the ID to the node counter
  id = nodecount;

  // Set all the child nodes to NULL
  for ( unsigned int i=0 ; i<4 ; ++i )
  {
    Child[i] = NULL;
  }

  // Set parent to NULL so it can be tested using if ( parent )
  Parent = NULL;

  // For Path routing
  A_Parent = NULL;
  cost = std::numeric_limits<double>::infinity();
  f_cost = std::numeric_limits<double>::infinity();
}

Node::~Node(void)
{
  // Node count decremented
  nodecount--;

  //type = NULL;
  id = NULL;
}


double Node::x_dsp()
{
  return abs( m_parentTree->get_left() - m_parentTree->get_right() ) / pow( 2.00 , depth );
}

double Node::y_dsp()
{
  return abs( m_parentTree->get_top() - m_parentTree->get_bottom() ) / pow( 2.00 , depth );
}

double Node::x_dsp( int depth_ )
{
  return abs( m_parentTree->get_left() - m_parentTree->get_right() ) / pow( 2.00 , depth_ );
}

double Node::y_dsp( int depth_ )
{
  return abs( m_parentTree->get_top() - m_parentTree->get_bottom() ) / pow( 2.00 , depth_ );
}

double Node::centre_x()
{
  if ( type == Node::ROOT_TYPE )
  {
    // in a vector
    return  m_parentTree->left +  m_parentTree->gridWidth - x_dsp();
  }

  else
  {
    if (this == Parent->Child[0]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_x() - ( x_dsp() ) 
          + ( 2 * m_parentTree->h_order[0] * - x_dsp() );
      
    }

    else if ( this == Parent->Child[1]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_x() - ( x_dsp() ) 
          + ( 2 * m_parentTree->h_order[1] * x_dsp() );

    }

    else if (this == Parent->Child[2]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_x() - ( x_dsp() ) 
          + ( 2 * m_parentTree->h_order[2] * x_dsp() );

    }

    else if (this == Parent->Child[3]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_x() - ( x_dsp() ) 
          + ( 2 * m_parentTree->h_order[3] * x_dsp() );
    }
    else
    {
      // Somthing wrong here!
      assert(false);
      return 0;
    }
  }
}

double Node::centre_y()
{
  if ( type == Node::ROOT_TYPE )
  {
    // in a vector
    return  m_parentTree->bottom +  m_parentTree->gridHeight - y_dsp();
  }

  else
  {
    if (this == Parent->Child[0]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_y() - ( y_dsp() ) 
          + ( 2 * m_parentTree->v_order[0] * y_dsp() );
    }

    else if ( this == Parent->Child[1]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_y() - ( y_dsp() ) 
          + ( 2 * m_parentTree->v_order[1] * y_dsp() );
    }

    else if (this == Parent->Child[2]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_y() - ( y_dsp() ) 
          + ( 2 * m_parentTree->v_order[2] * y_dsp() );
    }

    else if (this == Parent->Child[3]) 
    { 
      // Locate the centre point of the node;
      return  Parent->centre_y() - ( y_dsp() ) 
          + ( 2 * m_parentTree->v_order[3] * y_dsp() );
    }
    else
    {
      // Somthing wrong here!
      assert(false);
      return 0;
    }

  }
}


double Node::centre_z()
{
  return 0;
}
