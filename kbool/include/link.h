/*! \file kbool/include/kbool/link.h
    \brief Part of a graph, connection between nodes (Header)
    \author Probably Klaas Holwerda or Julian Smart

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: see kboollicense.txt 

    RCS-ID: $Id: link.h,v 1.1 2005/05/24 19:13:37 titato Exp $
*/

#ifndef LINK_H
#define LINK_H

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif

#include "kbool/include/booleng.h"
#include "kbool/include/_lnk_itr.h"

enum LinkStatus {IS_LEFT,IS_ON,IS_RIGHT};

class LPoint;
class Node;
class Record;

//!   segment within a graph 
/*
   A Graph contains a list of KBoolLink, the KBoolLink or connected by Node's. 
   Several KBoolLink can be connected to one Node. 
   A KBoolLink has a direction defined by its begin and end node.
   Node do have a list of connected KBoolLink's.
   So one can walk trough a graph in two ways:
   1- via its KBoolLink list 
   2- via the node connected to the KBoolLink's 
*/
class A2DKBOOLDLLEXP KBoolLink
{
   protected:
      Bool_Engine* _GC;
	public:

		//! contructors 
		KBoolLink(Bool_Engine* GC);

		//! contructors 
		KBoolLink(int graphnr, Node* begin, Node* end, Bool_Engine* GC);

		//! contructors 
		KBoolLink(Node *begin, Node *end, Bool_Engine* GC);

		//! destructors
		~KBoolLink();


      //! Merges the other node with argument
		void MergeNodes(Node* const);      				

      //! outproduct of two links
      LinkStatus OutProduct(KBoolLink* const two,double accur);              
		
      //! link three compared to this and two
      LinkStatus PointOnCorner(KBoolLink* const, KBoolLink* const);	

      //! Removes argument from the link
		void Remove(Node*);		
      
      //! replaces olddone in the link by newnode
		void Replace(Node* oldnode, Node* newnode);		

      //!top hole marking
		void SetTopHole(bool value);

      //!top hole marking
		bool IsTopHole();

      //! Marking functions
		void UnMark();
      //! Marking functions
		void Mark();
      //! Marking functions
		void SetMark(bool);
      //! Marking functions
		bool IsMarked();

      //! holelink Marking functions
      void SetHoleLink(bool val){ m_holelink = val;};

      //! holelink Marking functions
      bool GetHoleLink(){ return m_holelink;};

      //! Bin functions
		void SetNotBeenHere();
      //! Bin functions
		void SetBeenHere();
      //! Have you been here ??
		bool BeenHere();										

      //! Removes all the references to this
		void	UnLink();												

		//! functions for maximum performance
		Node*	GetBeginNode();      				  	

      //! Datamember access functions
		Node*	GetEndNode();
		Node*	GetLowNode();
		Node*	GetHighNode();

      //! Returns a next link beginning with argument
		KBoolLink* Forth(Node*); 	
      
		int GetGraphNum();
		bool GetInc();
      bool GetLeftA();
		bool GetLeftB();
		bool GetRightA();
		bool GetRightB();
		void GetLRO(LPoint*, int&, int&, double);

      //! Return a node not equal to arg.
		Node*	GetOther(const Node* const);			
      //! Is this link unused ?
		bool IsUnused();									

      //! Used for given operation ?
		bool IsMarked(BOOL_OP operation);            

      //! return true if Left side is marked true for operation
		bool IsMarkedLeft(BOOL_OP operation);            

      //! return true if Right side is marked true for operation
		bool IsMarkedRight(BOOL_OP operation);            

      //! is this a hole link for given operation
		bool IsHole(BOOL_OP operation);				
      
      //! set the hole mark
      void SetHole(bool);

      //! is the hole mark set?
		bool GetHole();

      //! Are the nodes on about the same coordinates ?
		bool IsZero(B_INT marge );		
		bool ShorterThan(B_INT marge );

      //! Resets the link
		void Reset(Node* begin, Node* end, int graphnr = 0);	
		void Set(Node* begin, Node* end);
		void SetBeginNode(Node*);
		void SetEndNode(Node*);
		void SetGraphNum(int);
		void SetInc(bool);
      void SetLeftA(bool);
		void SetLeftB(bool);
		void SetRightA(bool);
		void SetRightB(bool);
		void SetGroup(GroupType);
		GroupType Group();

      //! Flag calculation (internal only)
		void SetLineTypes();
		void Reset();
		void Reset_flags();

      //!put in this direction
      void Redirect(Node* a_node); 

      void TakeOverOperationFlags( KBoolLink* link );

      void SetRecordNode( DL_Node<Record*>* recordNode ) { m_record = recordNode; }

      DL_Node<Record*>* GetRecordNode() { return m_record; }

	protected:

      //! The mainitems of a link
		Node 	*m_beginnode, *m_endnode;
      //! Marker for walking over the graph
		bool	m_bin    	: 1;				
      //! Is this a part of hole ?
		bool	m_hole     : 1;				
      //! link that is toplink of hole?
		bool	m_hole_top : 1;				
      //! going in one more time in this graph if true  else going out one time
		bool 	m_Inc   	: 1;    			
      //! Is left in polygongroup A
      bool 	m_LeftA 	: 1;    			
      //! Is right in polygon group A
		bool 	m_RightA	: 1;    			
      //! Is left in polygon group B
		bool 	m_LeftB 	: 1;    			
      //! Is right in polygongroup B
		bool	m_RightB	: 1;    			
      //! General purose marker, internally unused
		bool	m_mark		: 1;    			
      //! link for linking holes
		bool	m_holelink : 1; 
   				
      //! Marker for Merge Left
		bool m_merge_L 	: 1;				
      //! Marker for substract a-b Left
		bool m_a_substract_b_L: 1;	  		
      //! Marker for substract b-a Left
		bool m_b_substract_a_L: 1;	  		
      //! Marker for intersect Left
		bool m_intersect_L: 1;				
      //! Marker for X-OR Left
		bool m_exor_L: 1;				

      //! Marker for Merge Right
		bool m_merge_R 	: 1;				
      //! Marker for substract a-b Right
		bool m_a_substract_b_R: 1;	  		
      //! Marker for substract b-a Right
		bool m_b_substract_a_R: 1;	  		
      //! Marker for intersect Right
		bool m_intersect_R: 1;				
      //! Marker for X-OR Right
		bool m_exor_R: 1;				

      //! belongs to group A or B
		GroupType m_group : 1;      
      
      //! belongs to this polygon part in the graph.
		int m_graphnum;

      DL_Node<Record*>* m_record; 
};

#endif

