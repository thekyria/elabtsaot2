/*!
\file sha1.h
\brief This class implements the Secure Hashing Standard as defined in FIPS PUB
180-1 published April 17, 1995.

Many of the variable names in this class, especially the single character names,
were used because those were the names used in the publication.

Please read the file sha1.cpp for more information.
\date 2009-06-22 19:34:25Z
\author Paul E. Jones, paulej at packetizer dot com
*/

#ifndef SHA1_H
#define SHA1_H

#include <cstdio>

namespace elabtsaot{

/*!
  \brief This class implements the Secure Hashing Standard as defined in FIPS
  PUB 180-1 published April 17, 1995.

  The Secure Hashing Standard, which uses the Secure Hashing Algorithm (SHA),
  produces a 160-bit message digest for a given data stream.  In theory, it is
  highly improbable that two messages will produce the same message digest.
  Therefore, this algorithm can serve as a means of providing a "fingerprint"
  for a message.

  SHA-1 is defined in terms of 32-bit "words".  This code was written with the
  expectation that the processor has at least a 32-bit machine word size. If the
  machine word size is larger, the code should still function properly. One
  caveat to that is that the input functions taking characters and character
  arrays assume that only 8 bits of information are stored in each character.

  \warning SHA-1 is designed to work with messages less than 2^64 bits long.
  Although SHA-1 allows a message digest to be generated for messages of any
  number of bits less than 2^64, this implementation only works with messages
  with a length that is a multiple of 8 bits.

  \example sha1_test.cpp This is an example on how to use the SHA1 class

  \author paulej
*/
class SHA1 {

 public:

  SHA1();
  virtual ~SHA1();

  //! Re-initialize the class
  /*!
    This function will initialize the sha1 class member variables in preparation
    for computing a new message digest.
  */
  void Reset();

  //! Returns the message digest
  /*!
    This function will return the 160-bit message digest into the array provided

    \param message_digest_array This is an array of five unsigned integers which
                                will be filled with the message digest that has
                                been computed
    \return True if successful, false if it failed
  */
  bool Result(unsigned* message_digest_array);

  /*! \name Input
    Provide input to SHA1
  */
  //@{

  //! This function accepts an array of octets as the next portion of the
  //! message.
  /*!
    \param message_array An array of characters representing the next portion of
                         the message
    \param length The length of the message_array
  */
  void Input( unsigned char const* message_array, size_t length);
  //! This function accepts an array of octets as the next portion of the
  //! message.
  /*!
    \param message_array An array of characters representing the next portion
                          of the message.
    \param length The length of the message_array
  */
  void Input( char const* message_array, size_t length);
  //! This function accepts a single octet as the next message element.
  /*!
    \param message_element The next octet in the message.
  */
  void Input( unsigned char message_element);
  //! This function accepts a single octet as the next message element.
  /*!
    \param message_element The next octet in the message.
  */
  void Input( char message_element);
  //! operator<<
  /*!
    This operator makes it convenient to provide character strings to the SHA1
    object for processing.

    Comment: Each character is assumed to hold 8 bits of information.

    \param message_array The character array to take as input.
    \return A reference to the SHA1 object.
  */
  SHA1& operator<<( char const* message_array);
  //! operator<<
  /*!
    This operator makes it convenient to provide character strings to the SHA1
    object for processing.

    Comment: Each character is assumed to hold 8 bits of information.

    \param message_array The character array to take as input.
    \return A reference to the SHA1 object.
  */
  SHA1& operator<<( unsigned char const* message_array);
  //! operator<<
  /*!
    This function provides the next octet in the message.

    Comment: The character is assumed to hold 8 bits of information.

    \param message_element The next octet in the message
    \return A reference to the SHA1 object.
   */
  SHA1& operator<<( char const message_element);
  //! operator<<
  /*!
    This function provides the next octet in the message.

    Comment: The character is assumed to hold 8 bits of information.

    \param message_element The next octet in the message
    \return A reference to the SHA1 object.
   */
  SHA1& operator<<( unsigned char const message_element);
  //@}

 private:

  //! Process the next 512 bits of the message
  /*!
    This function will process the next 512 bits of the message stored in the
    Message_Block array.

    Comments: Many of the variable names in this function, especially the single
    character names, were used because those were the names used in the
    publication.
  */
  void ProcessMessageBlock();
  //! Pads the current message block to 512 bits
  /*!
    According to the standard, the message must be padded to an even
    512 bits.  The first padding bit must be a '1'.  The last 64 bits
    represent the length of the original message.  All bits in between
    should be 0.  This function will pad the message according to those
    rules by filling the message_block array accordingly.  It will also
    call ProcessMessageBlock() appropriately.  When it returns, it
    can be assumed that the message digest has been computed.
  */
  void PadMessage();
  //! Performs a circular left shift operation
  /*!
    This member function will perform a circular shifting operation.

    \param bits The number of bits to shift (1-31)
    \param word The value to shift (assumes a 32-bit integer)
    \return The shifted value.
  */
  inline unsigned CircularShift(int bits, unsigned word);

  unsigned H[5];                      //!< Message digest buffers

  unsigned Length_Low;                //!< Message length in bits
  unsigned Length_High;               //!< Message length in bits

  unsigned char Message_Block[64];    //!< 512-bit message blocks
  int Message_Block_Index;            //!< Index into message block array

  bool Computed;                      //!< Is the digest computed?
  bool Corrupted;                     //!< Is the message digest corruped?

};

} // end of namespace elabtsaot

#endif // SHA1_H
