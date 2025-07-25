# automatically generated by the FlatBuffers compiler, do not modify

# namespace: flat

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Spot(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Spot()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSpot(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Spot
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Spot
    def Market(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Spot
    def Code(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Spot
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Spot
    def Datetime(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Spot
    def YesterdayClose(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Open(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def High(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Low(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Close(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Amount(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Volume(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float64Flags, o + self._tab.Pos)
        return 0.0

    # Spot
    def Bid(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Float64Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 8))
        return 0

    # Spot
    def BidAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Float64Flags, o)
        return 0

    # Spot
    def BidLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Spot
    def BidIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        return o == 0

    # Spot
    def BidAmount(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Float64Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 8))
        return 0

    # Spot
    def BidAmountAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Float64Flags, o)
        return 0

    # Spot
    def BidAmountLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Spot
    def BidAmountIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        return o == 0

    # Spot
    def Ask(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Float64Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 8))
        return 0

    # Spot
    def AskAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Float64Flags, o)
        return 0

    # Spot
    def AskLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Spot
    def AskIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        return o == 0

    # Spot
    def AskAmount(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Float64Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 8))
        return 0

    # Spot
    def AskAmountAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Float64Flags, o)
        return 0

    # Spot
    def AskAmountLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Spot
    def AskAmountIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        return o == 0

def SpotStart(builder):
    builder.StartObject(15)

def Start(builder):
    SpotStart(builder)

def SpotAddMarket(builder, market):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(market), 0)

def AddMarket(builder, market):
    SpotAddMarket(builder, market)

def SpotAddCode(builder, code):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(code), 0)

def AddCode(builder, code):
    SpotAddCode(builder, code)

def SpotAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    SpotAddName(builder, name)

def SpotAddDatetime(builder, datetime):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(datetime), 0)

def AddDatetime(builder, datetime):
    SpotAddDatetime(builder, datetime)

def SpotAddYesterdayClose(builder, yesterdayClose):
    builder.PrependFloat64Slot(4, yesterdayClose, 0.0)

def AddYesterdayClose(builder, yesterdayClose):
    SpotAddYesterdayClose(builder, yesterdayClose)

def SpotAddOpen(builder, open):
    builder.PrependFloat64Slot(5, open, 0.0)

def AddOpen(builder, open):
    SpotAddOpen(builder, open)

def SpotAddHigh(builder, high):
    builder.PrependFloat64Slot(6, high, 0.0)

def AddHigh(builder, high):
    SpotAddHigh(builder, high)

def SpotAddLow(builder, low):
    builder.PrependFloat64Slot(7, low, 0.0)

def AddLow(builder, low):
    SpotAddLow(builder, low)

def SpotAddClose(builder, close):
    builder.PrependFloat64Slot(8, close, 0.0)

def AddClose(builder, close):
    SpotAddClose(builder, close)

def SpotAddAmount(builder, amount):
    builder.PrependFloat64Slot(9, amount, 0.0)

def AddAmount(builder, amount):
    SpotAddAmount(builder, amount)

def SpotAddVolume(builder, volume):
    builder.PrependFloat64Slot(10, volume, 0.0)

def AddVolume(builder, volume):
    SpotAddVolume(builder, volume)

def SpotAddBid(builder, bid):
    builder.PrependUOffsetTRelativeSlot(11, flatbuffers.number_types.UOffsetTFlags.py_type(bid), 0)

def AddBid(builder, bid):
    SpotAddBid(builder, bid)

def SpotStartBidVector(builder, numElems):
    return builder.StartVector(8, numElems, 8)

def StartBidVector(builder, numElems):
    return SpotStartBidVector(builder, numElems)

def SpotAddBidAmount(builder, bidAmount):
    builder.PrependUOffsetTRelativeSlot(12, flatbuffers.number_types.UOffsetTFlags.py_type(bidAmount), 0)

def AddBidAmount(builder, bidAmount):
    SpotAddBidAmount(builder, bidAmount)

def SpotStartBidAmountVector(builder, numElems):
    return builder.StartVector(8, numElems, 8)

def StartBidAmountVector(builder, numElems):
    return SpotStartBidAmountVector(builder, numElems)

def SpotAddAsk(builder, ask):
    builder.PrependUOffsetTRelativeSlot(13, flatbuffers.number_types.UOffsetTFlags.py_type(ask), 0)

def AddAsk(builder, ask):
    SpotAddAsk(builder, ask)

def SpotStartAskVector(builder, numElems):
    return builder.StartVector(8, numElems, 8)

def StartAskVector(builder, numElems):
    return SpotStartAskVector(builder, numElems)

def SpotAddAskAmount(builder, askAmount):
    builder.PrependUOffsetTRelativeSlot(14, flatbuffers.number_types.UOffsetTFlags.py_type(askAmount), 0)

def AddAskAmount(builder, askAmount):
    SpotAddAskAmount(builder, askAmount)

def SpotStartAskAmountVector(builder, numElems):
    return builder.StartVector(8, numElems, 8)

def StartAskAmountVector(builder, numElems):
    return SpotStartAskAmountVector(builder, numElems)

def SpotEnd(builder):
    return builder.EndObject()

def End(builder):
    return SpotEnd(builder)
