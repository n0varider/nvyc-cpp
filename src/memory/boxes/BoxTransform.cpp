#include "BoxTransform.hpp"

using nvyc::BoxType;

namespace nvyc::Memory {

    /*
    
    Box movements
    OWNED <-> LOCKED
    OWNED -> GLASS
    
    returnbox([box]) to move it back
    
    */
    bool validTransform(BoxType alpha, BoxType beta) {
       switch(alpha) {
            case BoxType::OWNED:
                return (beta == BoxType::LOCKED || beta == BoxType::GLASS);
            case BoxType::LOCKED:
                return (beta == BoxType::OWNED);
            default:
                break;
       }
       return false;
    }

    /*
    
    OwnedBox    Owned, RWM
    GlassBox    R
    LockedBox   Owned, No Permissions
    EmptyBox    No Permissions
    
    */
    bool validOperations(BoxType box, int flags) {
        if(box == BoxType::OWNED) {
            // if flags ^ ALL == 0, !0 = 1
            return !(flags ^ (CAN_READ | CAN_WRITE | CAN_MOVE)); 
        }

        if(box == BoxType::LOCKED) return false;
        if(box == BoxType::GLASS) return !(flags ^ CAN_READ);
        if(box == BoxType::EMPTY) return false;
        return false;
    }

}