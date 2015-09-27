/* 
 * File:   SafetySynthesis.cpp
 * Author: Peter G. Jensen
 * 
 * Created on 14 September 2015, 15:59
 */
#include <assert.h>
#include <set>
#include "SafetySynthesis.h"
#include "../DataStructures/SimpleMarkingStore.h"
#include "../DataStructures/PTrieMarkingStore.h"

namespace VerifyTAPN {
namespace DiscreteVerification {
    
SafetySynthesis::SafetySynthesis(   TAPN::TimedArcPetriNet& tapn, 
                                    NonStrictMarking& initialMarking,
                                    AST::Query* query, 
                                    VerificationOptions& options)
                                : tapn(tapn), initial_marking(initialMarking),
                                    query(query), options(options), 
                                    placeStats(tapn.getNumberOfPlaces()),
                                    generator(tapn), discovered(0), explored(0),
                                    largest(0)
{
    if(options.getMemoryOptimization() == VerificationOptions::PTRIE)
        store = new PTrieMarkingStore<SafetyMeta>(tapn, options.getKBound());
    else 
        store = new SimpleMarkingStore<SafetyMeta>();
}

bool SafetySynthesis::satisfies_query(NonStrictMarkingBase* m)
{
    m->cut(placeStats);
    if(m->size() > options.getKBound()) return false;
    QueryVisitor<NonStrictMarkingBase> checker(*m, tapn);
    BoolResult context;        
    query->accept(checker, context);
    return context.value;
}

bool SafetySynthesis::run()
{
    stack_t waiting;
    largest = initial_marking.size();
    if(!satisfies_query(&initial_marking)) return false;
    
    store_t::result_t m_0_res = store->insert_and_dealloc(&initial_marking);

    SafetyMeta& meta = store->get_meta(m_0_res.second);
    meta = {UNKNOWN, false, false, 0, 0, depends_t()};
    meta.waiting = true;
    
    waiting.push(m_0_res.second);
        
    while(!waiting.empty() && meta.state != LOOSING && meta.state != WINNING)
    {
        ++explored;
        store_t::Pointer* next = waiting.top();
        waiting.pop();
        
        SafetyMeta& next_meta = store->get_meta(next);
//        std::cout   << "pop " << next << " State: " << &next_meta << " - > "
//                    << next_meta.state << std::endl;
        
        next_meta.waiting = false;
        if( next_meta.state == LOOSING ||
            next_meta.state == WINNING)
        {
            dependers_to_waiting(next_meta, waiting);
        }
        else
        {
            assert(next_meta.state == UNKNOWN);
            next_meta.state = PROCESSED;
            // generate successors for environment
            successors(next, next_meta, waiting, false);

            if(next_meta.state != LOOSING)
            {
                // generate successors for controller
                successors(next, next_meta, waiting, true);
            }
            
            if(next_meta.state == MAYBE_WINNING && next_meta.env_children == 0)
            {
                next_meta.state = WINNING;
            }
            
            if(next_meta.state == LOOSING || next_meta.state == WINNING)
            {
                assert(store->get_meta(next).state != PROCESSED);
                waiting.push(next);
            }
        }
        store->free(next);
    }
    return meta.state != LOOSING;
}

void SafetySynthesis::dependers_to_waiting(SafetyMeta& next_meta, stack_t& waiting)
{
    for(auto ancestor : next_meta.dependers)
    {
        SafetyMeta& a_meta = store->get_meta(ancestor.second);
        if(a_meta.state == LOOSING || a_meta.state == WINNING) continue;
        
        bool ctrl_child = ancestor.first;
        if(ctrl_child)
        {
            a_meta.ctrl_children -= 1;
            if(next_meta.state == WINNING)
            {
                a_meta.state = MAYBE_WINNING;
            } 
                        
            if(a_meta.ctrl_children == 0 && a_meta.state != MAYBE_WINNING)
                    a_meta.state = LOOSING;
            
        }
        else
        {
            a_meta.env_children -= 1;  
            if(next_meta.state == LOOSING) a_meta.state = LOOSING;
        }        
        
        if(a_meta.env_children == 0 && a_meta.state == MAYBE_WINNING)
        {
            a_meta.state = WINNING;
        }
        
        if(a_meta.state == WINNING || a_meta.state == LOOSING)
        {
//            if(a_meta.state == LOOSING) std::cout << "LOOSING : " << ancestor.second << std::endl;
            assert(store->get_meta(ancestor.second).state != PROCESSED);
            if(!a_meta.waiting) waiting.push(ancestor.second);
            a_meta.waiting = true;
        }
    }
    next_meta.dependers.clear();
}

void SafetySynthesis::successors(   store_t::Pointer* parent, 
                                    SafetyMeta& meta, 
                                    stack_t& waiting,
                                    bool is_controller)
{
    NonStrictMarkingBase* marking = store->expand(parent);
    generator.from_marking(marking, 
            is_controller ? 
                Generator::CONTROLLABLE : Generator::ENVIRONMENT,
            meta.urgent);
    
//    std::cout << (is_controller ? "controller" : "env ");
//    std::cout << marking << " : " << *marking << std::endl;
    
    NonStrictMarkingBase* next = NULL;
    std::set<store_t::Pointer*> successors;
    size_t number_of_children = 0;
    bool terminated = false;
    bool all_loosing = true;
    while((next = generator.next(is_controller)) != NULL)
    {  

        meta.urgent |= generator.urgent();
        largest = std::max(next->size(), largest);
        ++discovered;
        ++number_of_children;
        
//        std::cout << "\tchild  " <<  next << " : " << *next << std::endl;

        if(!satisfies_query(next)) 
        {
//            std::cout << "\t\tdoes not satisfy phi" << std::endl;
            delete next;
            
            if(is_controller) 
            {
                continue;
            }
            else
            {
                meta.state = LOOSING;
//                std::cout << "LOOSING : " << parent << std::endl;
                terminated = true;
                break;
            }
        }

        store_t::result_t res = store->insert_and_dealloc(next);

        store_t::Pointer* p = res.second;

        if(res.first)
        {
            SafetyMeta childmeta = {UNKNOWN, false, false, 0, 0, depends_t()};
            store->set_meta(p, childmeta);
            successors.insert(p);
            all_loosing = false;
//            std::cout << "\t\t" << p << std::endl;
            continue;
        }
               
        SafetyMeta& childmeta = store->get_meta(p);
        if(!is_controller && childmeta.state == LOOSING)
        {
            meta.state = LOOSING;
//            std::cout << "LOOSING : " << parent << std::endl;
            terminated = true;
            break;
        } 
        else if(is_controller && (childmeta.state == WINNING || p == parent))
        {
            meta.state = MAYBE_WINNING;
            terminated = true;
            break;
        }
        
        if(childmeta.state != WINNING && childmeta.state != LOOSING)
        {
            successors.insert(p);
        }
        all_loosing = all_loosing && (childmeta.state == LOOSING);
    }
    
    store->free(marking);
    
    if(terminated) return; // Add nothing to waiting, we already have result

    if(is_controller)
    {
        if(number_of_children == 0)
        {
            meta.state = MAYBE_WINNING;
            return;
        } 
        else if(all_loosing)
        {
            meta.state = LOOSING;
//            std::cout << "LOOSING : " << parent << std::endl;
            return;
        }
    }

    if(is_controller) meta.ctrl_children = successors.size();
    else meta.env_children = successors.size();
    
    for(auto child : successors)
    {
        SafetyMeta& childmeta = store->get_meta(child);
        childmeta.dependers.push_front(
                                            depender_t(is_controller,parent));
        if(childmeta.state == UNKNOWN && 
                !childmeta.waiting)
        {
            childmeta.waiting = true;  
            waiting.push(child);
        } 
    }    
}

void SafetySynthesis::print_stats() {
    std::cout << "  discovered markings:\t" << discovered << std::endl;
    std::cout << "  explored markings:\t" << explored << std::endl;
    std::cout << "  stored markings:\t" << store->size() << std::endl;
};

SafetySynthesis::~SafetySynthesis() {
    delete store;
}
}
}

