#pragma once
#include "data/NodeType.hpp"

using nvyc::data::NodeType;

struct DeleteStream {
    NodeType type;

    /*void operator()(void* p) const {
        if(!p) return;
        switch(p)
    }*/
};