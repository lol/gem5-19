#include "base/callback.hh"
#include "mem/ramulator.hh"
#include "Ramulator/src/Gem5Wrapper.h"
#include "Ramulator/src/Request.h"
#include "sim/system.hh"
#include "debug/Ramulator.hh"

// daz3
ramulator::Gem5Wrapper* wrapper1 = NULL;
bool del_wrapper = false;
Tick begin_tick = 0;
Tick print_interval = 200000000;//0.2ms
unsigned long my_read_cnt = 0;
unsigned long my_write_cnt = 0;
unsigned long my_total_cnt = 0;

Ramulator::Ramulator(const Params *p):
    AbstractMemory(p),
    port(name() + ".port", *this),
    requestsInFlight(0),
    drain_manager(NULL),
    config_file(p->config_file),
    configs(p->config_file),
    wrapper(NULL),
    read_cb_func(std::bind(&Ramulator::readComplete, this, std::placeholders::_1)),
    write_cb_func(std::bind(&Ramulator::writeComplete, this, std::placeholders::_1)),
    ticks_per_clk(0),
    resp_stall(false),
    req_stall(false),
    send_resp_event(this),
    tick_event(this) 
{
  warmuptime = p->real_warm_up;

  configs.set_core_num(p->num_cpus);
  configs.set_tracefile_directory(p->output_dir);
}
Ramulator::~Ramulator()
{
    // delete wrapper;
    // daz3
    if(del_wrapper == false)
    {
        delete wrapper;
        del_wrapper = true;
    }
}

void Ramulator::init() {
    if (!port.isConnected()){ 
        fatal("Ramulator port not connected\n");
    } else { 
        port.sendRangeChange(); 
    }

    if (wrapper1 != NULL)
    {
        wrapper = wrapper1;
    }
    else
    {
        wrapper = new ramulator::Gem5Wrapper(configs, system()->cacheLineSize());
        wrapper1 = wrapper;
    }
    // daz3
    // wrapper = new ramulator::Gem5Wrapper(configs, system()->cacheLineSize());
    ticks_per_clk = Tick(wrapper->tCK * SimClock::Float::ns);

    DPRINTF(Ramulator, "Instantiated Ramulator with config file '%s' (tCK=%lf, %d ticks per clk)\n", 
        config_file.c_str(), wrapper->tCK, ticks_per_clk);
    Callback* cb = new MakeCallback<ramulator::Gem5Wrapper, &ramulator::Gem5Wrapper::finish>(wrapper);
    registerExitCallback(cb);
}

void Ramulator::startup() {
    schedule(tick_event, clockEdge());
}

unsigned int Ramulator::drain(DrainManager* dm) {
    // DPRINTF(Ramulator, "Requested to drain\n");
    // // updated to include all in-flight requests
    // // if (resp_queue.size()) {
    // if (numOutstanding()) {
    //     setDrainState(Drainable::Draining);
    //     drain_manager = dm;
    //     return 1;
    // } else {
    //     setDrainState(Drainable::Drained);
    //     return 0;
    // }
    return 0;
}

Port& Ramulator::getPort(const std::string& if_name, PortID idx) {
    if (if_name != "port") {
        return AbstractMemory::getPort(if_name, idx);
    } else {
        return port;
    }
}

void Ramulator::sendResponse() {
    assert(!resp_stall);
    assert(!resp_queue.empty());

    DPRINTF(Ramulator, "Attempting to send response\n");

    long addr = resp_queue.front()->getAddr();
    if(addr){/*DO NOTHING. For avoid error unused-variable*/}
    if (port.sendTimingResp(resp_queue.front())){
        DPRINTF(Ramulator, "Response to %ld sent.\n", addr);
        resp_queue.pop_front();
        if (resp_queue.size() && !send_resp_event.scheduled())
            schedule(send_resp_event, curTick());

        // check if we were asked to drain and if we are now done
        if (drain_manager && numOutstanding() == 0) {
            drain_manager->signalDrainDone();
            drain_manager = NULL;
        }
    } else 
        resp_stall = true;
}
    
void Ramulator::tick() {
    wrapper->tick();
    if (req_stall){
        req_stall = false;
        port.sendRetryReq();
    }
    //AbstractMemory::occupancyL3Cache = L3->occupancy();
    schedule(tick_event, curTick() + ticks_per_clk);
}

// added an atomic packet response function to enable fast forwarding
Tick Ramulator::recvAtomic(PacketPtr pkt) {
    access(pkt);
    //L3->call(pkt->getAddr());
    // set an fixed arbitrary 50ns response time for atomic requests
    return pkt->cacheResponding() ? 0 : 50000;
}

void Ramulator::recvFunctional(PacketPtr pkt) {
    pkt->pushLabel(name());
    functionalAccess(pkt);
    for (auto i = resp_queue.begin(); i != resp_queue.end(); ++i)
        pkt->trySatisfyFunctional(*i);
    pkt->popLabel();
}

bool Ramulator::recvTimingReq(PacketPtr pkt) {
    // we should never see a new request while in retry
    assert(!req_stall);

    for (PacketPtr pendPkt: pending_del)
        delete pendPkt;
    pending_del.clear();

    // daz3
    if (begin_tick == 0) {
        begin_tick = curTick();
    }

    if (pkt->cacheResponding()) {
        // snooper will supply based on copy of packet
        // still target's responsibility to delete packet
        pending_del.push_back(pkt);
        return true;
    }

    // daz3
    if (curTick()<=(begin_tick + warmuptime))
      {
        my_total_cnt++;
        accessAndRespond(pkt);
        return true;
      }
    
    bool accepted = true;
    if (pkt->isRead()) {
      //DPRINTF(Ramulator, "context id: %d, thread id: %d\n", pkt->req->contextId(),
      //    pkt->req->threadId());
      ramulator::Request req(pkt->getAddr(), ramulator::Request::Type::READ, read_cb_func, pkt->req->isPrefetch(), 0);
        //daz3
        // ramulator::Request req(pkt->getAddr(), ramulator::Request::Type::READ, read_cb_func, 0);
        accepted = wrapper->send(req);
        if (accepted){
            reads[req.addr].push_back(pkt);
            DPRINTF(Ramulator, "Read to %ld accepted.\n", req.addr);

            // added counter to track requests in flight
            ++requestsInFlight;
            // daz3
            my_read_cnt++;
            my_total_cnt++;
        } else {
            req_stall = true;
        }
    } else if (pkt->isWrite()) {
        // Detailed CPU model always comes along with cache model enabled and
        // write requests are caused by cache eviction, so it shouldn't be
        // tallied for any core/thread
        ramulator::Request req(pkt->getAddr(), ramulator::Request::Type::WRITE, write_cb_func, false, 0);
        accepted = wrapper->send(req);
        if (accepted){
            accessAndRespond(pkt);
            DPRINTF(Ramulator, "Write to %ld accepted and served.\n", req.addr);

            // added counter to track requests in flight
            ++requestsInFlight;
            // daz3
            my_write_cnt++;
            my_total_cnt++;
        } else {
            req_stall = true;
        }
    } else {
        // keep it simple and just respond if necessary
        accessAndRespond(pkt);
        // daz3
        my_total_cnt++;
    }
    return accepted;
}

void Ramulator::recvRespRetry() {
    DPRINTF(Ramulator, "Retrying\n");

    assert(resp_stall);
    resp_stall = false;
    sendResponse();
}

void Ramulator::accessAndRespond(PacketPtr pkt) {
    bool need_resp = pkt->needsResponse();
    access(pkt);
    if (need_resp) {
        assert(pkt->isResponse());
        pkt->headerDelay = pkt->payloadDelay = 0;

        DPRINTF(Ramulator, "Queuing response for address %lld\n",
                pkt->getAddr());

        resp_queue.push_back(pkt);
	// gagan : added 18 ns latency for the L3 cache
        if (!resp_stall && !send_resp_event.scheduled())
            schedule(send_resp_event, curTick());
    } else 
        pending_del.push_back(pkt);
}

void Ramulator::readComplete(ramulator::Request& req){
    DPRINTF(Ramulator, "Read to %ld completed.\n", req.addr);
    auto& pkt_q = reads.find(req.addr)->second;
    PacketPtr pkt = pkt_q.front();
    pkt_q.pop_front();
    if (!pkt_q.size())
        reads.erase(req.addr);

    // added counter to track requests in flight
    --requestsInFlight;

    accessAndRespond(pkt);
}

void Ramulator::writeComplete(ramulator::Request& req){
    DPRINTF(Ramulator, "Write to %ld completed.\n", req.addr);

    // added counter to track requests in flight
    --requestsInFlight;

    // check if we were asked to drain and if we are now done
    if (drain_manager && numOutstanding() == 0) {
        drain_manager->signalDrainDone();
        drain_manager = NULL;
    }
}

Ramulator *RamulatorParams::create(){
    return new Ramulator(this);
}
