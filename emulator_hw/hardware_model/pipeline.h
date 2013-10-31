/*!
\file pipeline.h
\brief Definition file for class Pipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>
#include <utility>

namespace elabtsaot{

//! Pipeline class
/*!
  Base class used as parent by the calculation pipelines of the digital part
  (FPGA) of a hardware emulator slice.

  Generally, a pipeline performes calculations for different power systems
  elements. Elements to be calculated in the pipeline are positioned on the
  analog lattice (of an emulator slice) according to values of the vector
  _position.

  Elements in the pipeline are stored in rising pseudo_id order. Pseudo_id is a
  priority index calculated as:
  pseudo_id(ver_id,hor_id) = ver_id*hor_id_max + hor_id (see related func.)
  Elements in the pipeline are sorted with incresing pseudo_id, that is
  first in the pipeline would be stored an el. at pos (0,0), then (0,1) up
  to (0,hor_id_max-1) then (1,0) [yielding a pseudo_id = hor_id_max] (1,1)
  and so on.

  Actually a Pipeline object is never instantiated in the current
  implementation.

  \sa ConstILoadPipeline, ConstPLoadPipeline, ConstZLoadPipeline,
      GeneratorPipeline

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date June 2011
*/
class Pipeline {

 public:

  //! Constructor
  /*!
    \param element_capacity Capacity of the pipeline
    \param vertical_dim Vertical dimension of the emulator the pipeline is
                        instantiated for
    \param horizontal_dim Horizontal dimension of the emulator the pipeline is
                          instantiated for
  */
  Pipeline(size_t element_capacity, size_t vertical_dim, size_t horizontal_dim);
  //! Destructor
  virtual ~Pipeline(){}

  //! Resets the pipeline
  /*!
    The function is declared virtual as, for child classes of Pipeline, a
    specialized implementation should be provided.
  */
  virtual void reset();

  //! An element at a certain position is searched for
  /*!
    \param row vertical positioning of the searched for element
    \param col horizontal positioning of the searched for element

    \return array index of element in the _position array if search successful;
            -1 otherwise
  */
  int search_element(size_t row, size_t col) const;

  std::vector<std::string> stages() const;            //!< getter for _stages
  size_t element_count_max() const;             //!< getter for _element_count_max
  size_t element_count() const;                 //!< getter for _element_count
  size_t ver_id_max() const;                    //!< getter for _ver_id_max
  size_t hor_id_max() const;                    //!< getter for _hor_id_max
  std::vector<std::pair<int, int> > position() const; //!< getter for _position

  size_t calculate_pseudo_id(size_t row,size_t col) const;

 protected:

  //! Calculates the pseudo_id for a analog grid position
  /*! Pseudo_id is a priority index calculated as:
    pseudo_id(ver_id,hor_id) = ver_id*hor_id_max + hor_id

    Elements in the pipeline are sorted with incresing pseudo_id, that is
    first in the pipeline would be stored an el. at pos (0,0), then (0,1) up
    to (0,hor_id_max-1) then (1,0) [yielding a pseudo_id = hor_id_max] (1,1)
    and so on.

    \param row vertical positioning of the pseudo id to be calculated
    \param col horizontal positioning of the pseudo id to be calculated

    \return pseudo id resulting from the input arguments */

  std::vector<std::string> _stages; //!< Names of the pipeline stages
  size_t _elementCountMax; //!< Maximum element count in this pipeline
  size_t _elementCount;    //!< Current element count in this pipeline
  size_t _verIdMax;        //!< Maximum number of rows (topology)
  size_t _horIdMax;        //!< Maximum number of columns (topology)

  //! Position of the pipeline elements on the analog grid of the hw emulator
  /*! position[x] = (vertical pos, horizontal pos)
      <-1,-1> denotes no x'th element */
  std::vector<std::pair<int, int> > _position;

};

} // end of namespace elabtsaot

#endif // PIPELINE_H
