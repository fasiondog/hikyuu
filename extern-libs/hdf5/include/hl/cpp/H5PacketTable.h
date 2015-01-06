/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Packet Table wrapper classes
 *
 * Wraps the H5PT Packet Table C functions in C++ objects
 *
 * Nat Furrer and James Laird
 * February 2004
 */

#ifndef H5PTWRAP_H
#define H5PTWRAP_H

/* Public HDF5 header */
#include "hdf5.h"

#include "H5PTpublic.h"
#include "H5api_adpt.h"

class H5_HLCPPDLL  PacketTable
{
public:
    /* Null constructor
     * Sets table_id to "invalid"
     */
    PacketTable() {table_id = H5I_BADID;}

    /* "Open" Constructor
     * Opens an existing packet table, which can contain either fixed-length or
     * variable-length packets.
     */
    PacketTable(hid_t fileID, char* name);

    /* Destructor
     * Cleans up the packet table
     */
    ~PacketTable();

    /* IsValid
     * Returns true if this packet table is valid, false otherwise.
     * Use this after the constructor to ensure HDF did not have
     * any trouble making or opening the packet table.
     */
    bool IsValid();

#ifdef VLPT_REMOVED
    /* IsVariableLength
     * Return 1 if this packet table is a Variable Length packet table,
     * return 0 if it is Fixed Length.  Returns -1 if the table is
     * invalid (not open).
     */
    int IsVariableLength();
#endif /* VLPT_REMOVED */

    /* ResetIndex
     * Sets the "current packet" index to point to the first packet in the
     * packet table
     */
    void ResetIndex();

    /* SetIndex
     * Sets the current packet to point to the packet specified by index.
     * Returns 0 on success, negative on failure (if index is out of bounds)
     */
    int SetIndex(hsize_t index);

    /* GetIndex
     * Returns the position of the current packet.
     * On failure, returns 0 and error is set to negative.
     */
    hsize_t GetIndex(int& error);

    /* GetPacketCount
     * Returns the number of packets in the packet table.  Error
     * is set to 0 on success.  On failure, returns 0 and
     * error is set to negative.
     */
    hsize_t GetPacketCount(int& error);

    hsize_t GetPacketCount()
    {
        int ignoreError;
        return GetPacketCount(ignoreError);
    }

protected:
    hid_t table_id;
};

class H5_HLCPPDLL FL_PacketTable : virtual public PacketTable
{
public:
    /* Constructor
     * Creates a packet table in which to store fixed length packets.
     * Takes the ID of the file the packet table will be created in, the name of
     * the packet table, the ID of the datatype of the set, the size
     * of a memory chunk used in chunking, and the desired compression level
     * (0-9, or -1 for no compression).
     */
    FL_PacketTable(hid_t fileID, char* name, hid_t dtypeID, hsize_t chunkSize, int compression = -1);

    /* "Open" Constructor
     * Opens an existing fixed-length packet table.
     * Fails if the packet table specified is variable-length.
     */
    FL_PacketTable(hid_t fileID, char* name);

    /* AppendPacket
     * Adds a single packet to the packet table.  Takes a pointer
     * to the location of the data in memory.
     * Returns 0 on success, negative on failure
     */
    int AppendPacket(void * data);

    /* AppendPackets (multiple packets)
     * Adds multiple packets to the packet table.  Takes the number of packets
     * to be added and a pointer to their location in memory.
     * Returns 0 on success, -1 on failure.
     */
    int AppendPackets(size_t numPackets, void * data);

    /* GetPacket (indexed)
     * Gets a single packet from the packet table.  Takes the index
     * of the packet (with 0 being the first packet) and a pointer
     * to memory where the data should be stored.
     * Returns 0 on success, negative on failure
     */
    int GetPacket(hsize_t index, void * data);

    /* GetPackets (multiple packets)
     * Gets multiple packets at once, all packets between
     * startIndex and endIndex inclusive.  Also takes a pointer to
     * the memory where these packets should be stored.
     * Returns 0 on success, negative on failure.
     */
    int GetPackets(hsize_t startIndex, hsize_t endIndex, void * data);

    /* GetNextPacket (single packet)
     * Gets the next packet in the packet table.  Takes a pointer to
     * memory where the packet should be stored.
     * Returns 0 on success, negative on failure.  Index
     * is not advanced to the next packet on failure.
     */
    int GetNextPacket(void * data);

    /* GetNextPackets (multiple packets)
     * Gets the next numPackets packets in the packet table.  Takes a
     * pointer to memory where these packets should be stored.
     * Returns 0 on success, negative on failure.  Index
     * is not advanced on failure.
     */
    int GetNextPackets(size_t numPackets, void * data);
};

#ifdef VLPT_REMOVED
class H5_HLCPPDLL  VL_PacketTable : virtual public PacketTable
{
public:
    /* Constructor
     * Creates a packet table in which to store variable length packets.
     * Takes the ID of the file the packet table will be created in, the name of
     * the packet table, and the size of a memory chunk used in chunking.
     */
    VL_PacketTable(hid_t fileID, char* name, hsize_t chunkSize);

    /* "Open" Constructor
     * Opens an existing variable-length packet table.
     * Fails if the packet table specified is fixed-length.
     */
    VL_PacketTable(hid_t fileID, char* name);

    /* AppendPacket
     * Adds a single packet of any length to the packet table.
     * Takes a pointer to the location of the data in memory and the length of the data
     * in bytes.
     * Returns 0 on success, negative on failure.
     */
    int AppendPacket(void * data, size_t length);

    /* AppendPackets (multiple packets)
     * Adds multiple variable-length packets to the packet table.  Takes the
     * number of packets to be added and a pointer to an array of
     * hvl_t structs in memory.
     * Returns 0 on success, negative on failure.
     */
    int AppendPackets(size_t numPackets, hvl_t * data);

    /* GetPacket (indexed)
     * Gets a single variable-length packet from the packet table.  Takes
     * the index of the packet (with 0 being the first packet) and a pointer
     * to a hvl_t struct in which to store the packet's size and location.
     * Returns 0 on success, negative on failure.
     */
    int GetPacket(hsize_t index, hvl_t * data);

    /* GetPackets (multiple packets)
     * Gets multiple variable-length packets at once, all packets between
     * startIndex and endIndex inclusive.  Takes a pointer to an array
     * of hvl_t structs in memory in which to store pointers to the packets.
     * Returns 0 on success, negative on failure.
     */
    int GetPackets(hsize_t startIndex, hsize_t endIndex, hvl_t * data);

    /* GetNextPacket (single packet)
     * Gets the next packet in the packet table.  Takes a pointer to
     * an hvl_t struct where the packet should be stored.
     * Returns 0 on success, negative on failure.  Index
     * is not advanced to the next packet on failure.
     */
    int GetNextPacket(hvl_t * data);

    /* GetNextPackets (multiple packets)
     * Gets the next numPackets packets in the packet table.  Takes a
     * pointer to an array of hvl_t structs where pointers to the packets
     * should be stored.
     * Returns 0 on success, negative on failure.  Index
     * is not advanced on failure.
     */
    int GetNextPackets(size_t numPackets, hvl_t * data);

    /* FreeReadbuff
     * Frees the buffers created when variable-length packets are read.
     * Takes the number of hvl_t structs to be freed and a pointer to their
     * location in memory.
     * Returns 0 on success, negative on error.
     */
    int FreeReadbuff(size_t numStructs, hvl_t * buffer);
};
#endif /* VLPT_REMOVED */

#endif /* H5PTWRAP_H */
