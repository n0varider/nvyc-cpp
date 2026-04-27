#pragma once

#include "BoxType.hpp"

using nvyc::BoxType;


namespace nvyc::Memory {

    static constexpr int CAN_READ   = 1 << 0;
    static constexpr int CAN_WRITE  = 1 << 1;
    static constexpr int CAN_MOVE   = 1 << 2;

    bool validTransform(BoxType alpha, BoxType beta);
    bool validOperations(BoxType box, int flags);
    // void transferBox(BoxType ownerBoxType);

}