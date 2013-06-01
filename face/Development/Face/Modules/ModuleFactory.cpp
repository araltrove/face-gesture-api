#include "ModuleFactory.h"
#include "Tracer.h"
#include "Source.h"
#include "Message.h"
#include "ImageWrapper.h"

using namespace std;
using namespace tbb::flow;

ModuleFactory* ModuleFactory::instance_ = NULL;
tbb::mutex ModuleFactory::mutex_;


ModuleFactory* ModuleFactory::GetInstance(void)
{
    if(instance_ == NULL)
    {
        tbb::mutex::scoped_lock lock(mutex_);
        if(instance_ == NULL)
            instance_ = new ModuleFactory();
    }
    return instance_;
}


ModuleFactory::ModuleFactory(void)
:   source_(NULL)
{
}


ModuleFactory::~ModuleFactory(void)
{
    delete source_;
}


struct square 
{
    Message* operator()(Message* v) 
    {
        ImageWrapper* frameIn = dynamic_cast<ImageWrapper*>(v);
        cv::Mat frameInMat = frameIn->Rgb();

        
        for(int i = 0; i < 10000000; i++)
            sqrt(sqrt(sqrt(double(i)))) * sqrt(sqrt(sqrt(double(i)))) * sqrt(sqrt(sqrt(double(i))));

        IMSHOW("square", frameInMat);
        TRACE("square: " + frameIn->GetMetaData().GetFrameNumber());
        //Sleep(5000);
        return v; 
    }
};


void ModuleFactory::CreateConnections(const ConnectionMap& modules)
{
    tbb::flow::function_node<Message*, Message*>* funcNode;
    for(size_t i = 0; i < modules.size(); i++)
    {
        const PortNameParser& ppp = modules[i].first;

        if(ppp.GetModuleName() == "Source")
        {
            source_ = new Source(graph_, ppp.GetModuleName(), ppp.GetInstanceName());
            funcNode = new tbb::flow::function_node<Message*, Message*>(graph_, queueing, square());
            Source::SourceNodeType* sourceNode = source_->GetNode();
            make_edge(*sourceNode, *funcNode);
        }
        
        //FunctionNode* B = new FunctionNode(graph_, unlimited, square());
        //make_edge(*sourceScheduler_, *B);
        //sourceNodes_.push_back(A);
    }
}


void ModuleFactory::Start(void)
{
    graph_.wait_for_all();
}