
#include "graph.h"
using namespace elabtsaot;

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/make_connected.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/is_kuratowski_subgraph.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>
#include <boost/graph/is_straight_line_drawing.hpp>

//#include <vector>
using std::vector;

// Struct to hold coordinates of the planar embedding
struct coord_t{
  std::size_t x;
  std::size_t y;
};

std::vector< std::pair<int,int> >
graph::planarizeGraph( int vertexCount,
                       vector< std::pair<int,int> > e ){
  using namespace boost;
  using std::pair;

  int k = 0;
  // ---------- Check if planarization is trivial ----------
  if ( vertexCount < 3 ){
    vector< pair<int,int> > ans;
    for ( k = 0 ; k != vertexCount ; ++k )
      ans.push_back( pair<int,int>( k, 1) );
    return ans;
  }

  // ---------- Populate the graph ----------
  typedef adjacency_list< vecS, vecS, undirectedS,
                          property<vertex_index_t, int>,
                          property<edge_index_t, int>
                        > graph;
  graph g(vertexCount);
  // Populate graph with edges
  for( k = 0; k != static_cast<int>(e.size()); ++k )
    add_edge( e[k].first, e[k].second, g );
//   // Initialize the interior vertex index - WARNING! DEFECTIVE!
//  property_map<graph, vertex_index_t>::type v_index = get(vertex_index, g);
//  graph_traits<graph>::vertices_size_type vertex_count = 0;
  graph_traits<graph>::vertex_iterator  vi, vi_end;
//  for ( tie(vi, vi_end) = vertices(g); vi != vi_end ; ++vi )
//    put( v_index, *vi, vertex_count++ );
//  k = 0;
//  std::std::cout << "Original graph vertices:" << std::endl;
//  for ( tie(vi, vi_end) = vertices(g); vi != vi_end ; ++vi ){
//    std::std::cout << " v[" << k++ << "]: " << *vi << " " << std::endl;
////    std::cout << " vertex extId=" << v_index[*vi] << " ";
//  }
//  std::cout << std::endl;
  // Initialize the interior edge index
  property_map<graph, edge_index_t>::type e_index = get(edge_index, g);
  graph_traits<graph>::edges_size_type edge_count = 0;
  graph_traits<graph>::edge_iterator ei, ei_end;
  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei )
    put( e_index, *ei, edge_count++ );
//  k = 0;
//  std::cout << "Original graph edges (" << num_edges(g) << "):" << std::endl;
//  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
//    std::cout << " e[" << k++ << "]: " << *ei << " " << std::endl;
////    std::cout << " edge extId=" << e_index[*ei] << " ";
//  }
//  std::cout << std::endl;

  // ---------- Ensure that the graph is connected ----------
  vector< graph_traits<graph>::vertices_size_type > component(num_vertices(g));
  vector< graph_traits<graph>::vertices_size_type >::value_type connectedComponents;
  connectedComponents = connected_components(g, &component[0]);
//  std::cout << "Connected components count of the graph: ";
//  std::cout << connectedComponents << std::endl;

  if ( static_cast<int>(connectedComponents) != 1 ){
    make_connected(g);

//    // Update the number of the connected components
//    connectedComponents = connected_components(g, &component[0]);
//    std::cout << "Connected compts count of the graph after make_connected: ";
//    std::cout << connectedComponents << std::endl;

//    k = 0;
//    std::cout << "Connected graph edges (" <<num_edges(g)<< "):" << std::endl;
//    for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
//      std::cout << " e[" << k++ << "]: " << *ei << " " << std::endl;
//  //    std::cout << " edge extId=" << e_index[*ei] << " ";
//    }
//    std::cout << std::endl;

    // Renew edge indexes - as some new edges have been added
    edge_count = 0;
    for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei )
      put( e_index, *ei, edge_count++ );
  }

  // ---------- Create the planar embedding ----------
  /*A convenient way to separate the actual planarity test from algorithms that
    accept a planar graph as input is through an intermediate structure called a
    planar embedding. Instead of specifying the absolute positions of the
    vertices and edges in the plane as a plane drawing would, a planar embedding
    specifies their positions relative to one another. A planar embedding
    consists of a sequence, for each vertex in the graph, of all of the edges
    incident on that vertex in the order in which they are to be drawn around
    that vertex. The orderings defined by this sequence can either represent a
    clockwise or counter-clockwise iteration through the neighbors of each
    vertex, but the orientation must be consistent across the entire embedding*/
  typedef vector<vector< graph::edge_descriptor> > embedding_storage_t;
  typedef iterator_property_map< embedding_storage_t::iterator,
                                 property_map<graph, vertex_index_t>::type
                                > embedding_t;
  embedding_storage_t embedding_storage( num_vertices(g) );
  embedding_t embedding( embedding_storage.begin(), get(vertex_index, g) );
  typedef std::vector< graph::edge_descriptor > kuratowski_edges_t;
  kuratowski_edges_t kuratowski_edges;

  // Run Boyer-Myrvold planarity test
  bool is_planar = boyer_myrvold_planarity_test
      (
    boyer_myrvold_params::graph = g,
    boyer_myrvold_params::embedding = embedding,
    boyer_myrvold_params::kuratowski_subgraph = back_inserter(kuratowski_edges)
      );

  if ( !is_planar ){
//    std::cout << "The graph is not planar." << std::endl;
//    std::cout << "Kuratowski subgraph edges (" << kuratowski_edges.size();
//    std::cout << "/" << num_edges(g) << "):" << std::endl;

    kuratowski_edges_t::iterator ki, ki_end;
    ki_end = kuratowski_edges.end();
//    k = 0;
    for ( ki = kuratowski_edges.begin(); ki != ki_end; ++ki){
//        std::cout << "k_e[" << k++ << "]: " << *ki << std::endl;
    }
//    std::cout << std::endl;

    bool isKuratowskiSub = is_kuratowski_subgraph( g,
                                                   kuratowski_edges.begin(),
                                                   kuratowski_edges.end() );
    if ( isKuratowskiSub ){
//      std::cout << "Subgraph positively identified "
//                << "as a Kuratowski subgraph." <<std::endl;
    } else{
      std::cout << "Abnormal situation!" << std::endl;
      std::cout << "Subgraph could not be identified "
                << "as a Kuratowski subgraph!" << std::endl;
    }

    // Remove last edge in Kuratowski subgraph
    remove_edge( *(--ki), g );
    // Renew edge indexes - as some extra edges have been added
    edge_count = 0;
    for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei )
      put( e_index, *ei, edge_count++ );
//    k = 0;
//    std::cout << "Reduced graph edges (" << num_edges(g) << "):" << std::endl;
//    for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
//      std::cout << " e[" << k++ << "]: " << *ei << " " << std::endl;
//    }
//    std::cout << std::endl;

    int vertexCount_new = num_vertices(g);
    vector<pair<int,int> > e_new;
    for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
      int fr = static_cast<int>(source(*ei, g));
      int to = static_cast<int>(target(*ei, g));
      e_new.push_back( pair<int,int>(fr,to) );
    }
//    std::cout << "Reduced graph as function arguments:" << std::endl;
//    std::cout << " vertexCount_new = " << vertexCount_new << std::endl;
//    display( e_new );
//    std::cout << std::endl;

    return planarizeGraph( vertexCount_new, e_new );
  }
//  std::cout << "The graph is planar." << std::endl;
//  std::cout << "Original graph planar embedding:" << std::endl;
//  for ( size_t k = 0 ; k != embedding_storage.size() ; ++k ){
//    std::cout << " embedding_storage[" << k << "]: ";
//    for ( size_t m = 0 ; m != embedding_storage[k].size() ; ++m ){
//      std::cout << embedding_storage[k][m] << " ";
//    }
//    std::cout << std::endl;
//  }
//  std::cout << std::endl;

  // ---------- Make graph biconnected planar ----------
  make_biconnected_planar( g, embedding);

  // Renew edge indexes - as some extra edges have been added
  edge_count = 0;
  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei )
    put( e_index, *ei, edge_count++ );
//  k = 0;
//  std::cout << "Biconnected planar graph vertices:" << std::endl;
//  for ( tie(vi, vi_end) = vertices(g); vi != vi_end ; ++vi ){
//    std::cout << " v[" << k++ << "]: " << *vi << " " << std::endl;
//  }
//  std::cout << std::endl;
//  k = 0;
//  std::cout << "Biconnected planar graph edges ("
//            << num_edges(g) << "):" << std::endl;
//  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
//    std::cout << " e[" << k++ << "]: " << *ei << " " << std::endl;
//  }
//  std::cout << std::endl;

  // Renew plannar embedding - according to the new biconnected planar graph
  is_planar = boyer_myrvold_planarity_test (
    boyer_myrvold_params::graph = g,
    boyer_myrvold_params::embedding = embedding );
  if ( !is_planar ){
    // Should never happen
    std::cout << "Abnormal situation!";
    std::cout << "Originally planar graph not planar after made biconnected planar!";

    std::cout << std::endl << "Aborting ..." << std::endl;
    vector< pair<int,int> > temp;
    return temp;
  }
//  std::cout << "The graph is still planar." << std::endl;
//  std::cout << "Biconnected planar graph planar embedding:" << std::endl;
//  for ( size_t k = 0 ; k != embedding_storage.size() ; ++k ){
//    std::cout << " embedding_storage[" << k << "]: ";
//    for ( size_t m = 0 ; m != embedding_storage[k].size() ; ++m ){
//      std::cout << embedding_storage[k][m] << " ";
//    }
//    std::cout << std::endl;
//  }
//  std::cout << std::endl;

  // ---------- Make graph maximal planar ----------
  make_maximal_planar( g, embedding);

  // Renew edge indexes - as some extra edges have been added
  edge_count = 0;
  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei )
    put( e_index, *ei, edge_count++ );
//  k = 0;
//  std::cout << "Maximal planar graph vertices:" << std::endl;
//  for ( tie(vi, vi_end) = vertices(g); vi != vi_end ; ++vi ){
//    std::cout << " v[" << k++ << "]: " << *vi << " " << std::endl;
//  }
//  std::cout << std::endl;
//  k = 0;
//  std::cout << "Maximal planar graph edges ("
//            << num_edges(g) << "):" << std::endl;
//  for ( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
//    std::cout << " e[" << k++ << "]: " << *ei << " " << std::endl;
//  }
//  std::cout << std::endl;

  // Renew plannar embedding - according to the new maximal planar graph
  is_planar = boyer_myrvold_planarity_test (
    boyer_myrvold_params::graph = g,
    boyer_myrvold_params::embedding = embedding );
  if ( !is_planar ){
    // Should never happen
    std::cout << "Abnormal situation!";
    std::cout << "Originally planar graph, not planar after made maximal planar!";

    std::cout << std::endl << "Aborting ..." << std::endl;
    vector< pair<int,int> > temp;
    return temp;
  }
//  std::cout << "The graph is still planar." << std::endl;
//  std::cout << "Maximal planar graph planar embedding:" << std::endl;
//  for ( size_t k = 0 ; k != embedding_storage.size() ; ++k ){
//    std::cout << " embedding_storage[" << k << "]: ";
//    for ( size_t m = 0 ; m != embedding_storage[k].size() ; ++m ){
//      std::cout << embedding_storage[k][m] << " ";
//    }
//    std::cout << std::endl;
//  }
//  std::cout << std::endl;

  // ---------- Find a canonical ordering ----------
  typedef vector< graph::vertex_descriptor > ordering_storage_t;
  ordering_storage_t ordering;
  planar_canonical_ordering( g, embedding, back_inserter(ordering) );

//  ordering_storage_t::iterator oi;
//  std::cout << "Planar canonical ordering:" << std::endl;
//  k = 0;
//  for ( oi = ordering.begin() ; oi != ordering.end() ; ++oi )
//    std::cout << " ordering[" << k++ << "]: " << *oi << std::endl;
//  std::cout << std::endl;

  // ----- Perform Chrobak-Payne straigh line drawing -----
  // Set up a property map to hold the mapping from vertices to coord_t's
  typedef vector< coord_t > straight_line_drawing_storage_t;
  typedef iterator_property_map< straight_line_drawing_storage_t::iterator,
                                 property_map< graph, vertex_index_t >::type
                               > straight_line_drawing_t;
  straight_line_drawing_storage_t
      straight_line_drawing_storage( num_vertices(g) );
  straight_line_drawing_t
      straight_line_drawing( straight_line_drawing_storage.begin(),
                             get(vertex_index,g) );
  // Chrobak-Payne Straight Line Drawing can be performed (vertexCount >= 3)
  chrobak_payne_straight_line_drawing( g,
                                       embedding,
                                       ordering.begin(), ordering.end(),
                                       straight_line_drawing );
//  // Verify that the drawing is actually a plane drawing
//  bool isStraightLine = is_straight_line_drawing(g, straight_line_drawing);
//  if (  !isStraightLine ){
//    // Should never happen
//    std::cout << "Abnormal situation!";
//    std::cout << "Chrobak-Payne result not a straight line drawing!";

//    std::cout << std::endl << "Aborting ..." << std::endl;
//    vector< pair<int,int> > temp;
//    return temp;
//  }

  // Return result
  vector< pair<int,int> > ans;
  for( tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi){
    coord_t coord(get(straight_line_drawing,*vi));
    ans.push_back( pair<int,int>(coord.x, coord.y) );
  }

  return ans;
}

int graph::countIslands( Powersystem const& pws ){

  typedef boost::adjacency_list< boost::vecS, boost::vecS,
                                 boost::undirectedS > graph;
  graph Gr;

  // Fill the graph (adjacency list) used to check about islands in the topology
  // ---------- Initialize topology graph from powersystem ----------
  size_t branchCount = pws.getBrSet_size();
  for( size_t k = 0 ; k != branchCount ; ++k ){
    Branch const* br = pws.getBranch(k);
    if( !br ) break;

    int fr = pws.getBus_intId( br->fromBusExtId() );
    int to = pws.getBus_intId( br->toBusExtId() );
//    if( fr<0 || to<0 ) return false;

    boost::add_edge( fr, to, Gr );
  }

  // Run boost::connected_components algorithm
  vector< boost::graph_traits<graph>::vertices_size_type >
      component(num_vertices(Gr));
  vector< boost::graph_traits<graph>::vertices_size_type >::value_type
      connectedComponents;
  connectedComponents = boost::connected_components(Gr, &component[0]);

  return static_cast<int>(connectedComponents);

  // Alternatively
//  // component[k] is the island in which bus with intId k belongs to
//  vector<int> component(pws.getBusSet_size());
//  int num = boost::connected_components(Gr, &component[0]);
//  // Check for single-bus islands in the end of the _busSet
//  if ( boost::num_vertices(Gr) < pws.getBusSet_size() ){
//    for (k = boost::num_vertices(Gr) ; k != pws.getBusSet_size() ; ++k ){
//      component[k] = num++;
//    }
//  }
//  return num;
}

bool graph::isConnected( int vertexCount, std::vector<std::pair<int,int> > e ){

  using namespace boost;

  // ---------- Populate the graph ----------
  typedef adjacency_list< vecS, vecS, undirectedS,
                          property<vertex_index_t, int>,
                          property<edge_index_t, int>
                        > graph;
  graph g(vertexCount);
  // Populate graph with edges
  for( size_t k = 0 ; k != e.size() ; ++k )
    add_edge( e[k].first, e[k].second, g );

  // ---------- Ensure that the graph is connected ----------
  vector< graph_traits<graph>::vertices_size_type > component(num_vertices(g));
  vector< graph_traits<graph>::vertices_size_type >::value_type connectedComponents;
  connectedComponents = connected_components(g, &component[0]);
  if ( static_cast<int>(connectedComponents) != 1 )
    // If graph is not connected it is considered not planar (by convention)
    return false;
  else
    return true;
}

bool graph::isPlanar( Powersystem const& pws ){

  // ---------- Initialize topology graph from powersystem ----------
  std::vector<std::pair<int,int> > edges;
  size_t branchCount = pws.getBrSet_size();
  for( size_t k = 0 ; k != branchCount ; ++k ){

    // Get next branch
    Branch const* br = pws.getBranch(k);
    if( !br )
      break;

    int fr = pws.getBus_intId( br->fromBusExtId() );
    int to = pws.getBus_intId( br->toBusExtId() );
//    if( fr<0 || to<0 ) return false;

    // Add edge
    std::pair<int,int> e = std::pair<int,int>(fr,to);
    edges.push_back( e );
  }
  // Count how many buses exist in the network
  size_t vertexCount = pws.getBusSet_size();
  if ( vertexCount <= 0 )
    return false; // Planarization of an empty powersystem is trivial!

  return graph::isPlanar(vertexCount, edges);
}

bool graph::isPlanar( int vertexCount, std::vector<std::pair<int,int> > e ){

  using namespace boost;

  // ---------- Populate the graph ----------
  typedef adjacency_list< vecS, vecS, undirectedS,
                          property<vertex_index_t, int>,
                          property<edge_index_t, int>
                        > graph;
  graph g(vertexCount);
  // Populate graph with edges
  for( size_t k = 0 ; k != e.size() ; ++k )
    add_edge( e[k].first, e[k].second, g );

  // ---------- Ensure that the graph is connected ----------
  vector< graph_traits<graph>::vertices_size_type > component(num_vertices(g));
  vector< graph_traits<graph>::vertices_size_type >::value_type connectedComponents;
  connectedComponents = connected_components(g, &component[0]);
  if ( static_cast<int>(connectedComponents) != 1 )
    // If graph is not connected it is considered not planar (by convention)
    return false;

  typedef std::vector<std::vector<graph::edge_descriptor> > embedding_storage_t;
  typedef iterator_property_map< embedding_storage_t::iterator,
                                 property_map<graph, vertex_index_t>::type
                                > embedding_t;
  embedding_storage_t embedding_storage( num_vertices(g) );
  embedding_t embedding( embedding_storage.begin(), get(vertex_index, g) );
  typedef std::vector< graph::edge_descriptor > kuratowski_edges_t;
  kuratowski_edges_t kuratowski_edges;

  // ---------- Run Boyer-Myrvold planarity test ----------
  bool is_planar = boyer_myrvold_planarity_test
      (
    boyer_myrvold_params::graph = g,
    boyer_myrvold_params::embedding = embedding,
    boyer_myrvold_params::kuratowski_subgraph = back_inserter(kuratowski_edges)
      );

  return is_planar;
}
