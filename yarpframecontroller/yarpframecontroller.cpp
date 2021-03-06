#include <yarp/dev/IJoypadController.h>
#include <yarp/dev/IFrameTransform.h>
#include <yarp/os/RFModule.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/LogStream.h>
#include <yarp/sig/Matrix.h>
#include <yarp/math/FrameTransform.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include "ParamParser.h"
#include <math.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::math;

constexpr size_t axisCount   = 2;
constexpr size_t stickCount  = 2;
constexpr size_t buttonCount = 11;
constexpr double rate        = 0.01;

class FrameController : public RFModule
{
public:
    PolyDriver         joypadPD;
    PolyDriver         tfPD;
    IFrameTransform*   tfPublisher;
    IJoypadController* joypad;
    Matrix             left;
    Matrix             right;
    double             maxVelocity; //m/s
    double             velocity;   //velocity per cycle;
    double             axisVector[axisCount];
    Vector             sticks[2];
    float              buttonVector[buttonCount];
    FrameTransform     leftFrame;
    FrameTransform     rightFrame;
    Matrix             leftInitM;
    Matrix             rightInitM;
    double             limit[3]; /* Min and max limits for the frame position (w.r.t. to the initial position, default +inf )*/
    ParamParser        param{"framecontroller"};
    bool               invertPOV;

    FrameController() {}
    virtual double getPeriod() override { return rate; }
    
    virtual bool configure(ResourceFinder& rf) override
    {
        if (rf.check("help"))
        {
            yInfo() << "parameters:\n"                                                                  <<
                       "velocity           - double - velocity in m/s \n"                               <<
                       "rootFrame          - string - the source frame id for left and right frames \n" <<
                       "leftFrameInitial   - string - frame id of the actual left hand frame \n"        <<
                       "rightFrameInitial  - string - frame id of the actual right hand frame \n"       <<
                       "leftFrame          - string - target frame id for left hand \n"                 <<
                       "rightFrame         - string - target frame id for right hand \n"                <<
                       "invertPOV          - bool   - command from a frontal POV \n";
            return false;
        }
        unsigned int count;
        Property     cfg;
        Bottle       b;
        if (!param.parse(rf, "velocity",          ParamParser::TYPE_DOUBLE)) return false;
        if (!param.parse(rf, "rootFrame",         ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "leftFrameInitial",  ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "rightFrameInitial", ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "leftFrame",         ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "rightFrame",        ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "remote",            ParamParser::TYPE_STRING)) return false;
        if (!param.parse(rf, "invertPOV",         ParamParser::TYPE_BOOL  )) return false;

        maxVelocity             = rf.find("velocity").asDouble();
        velocity                = maxVelocity * rate;
        leftFrame.dst_frame_id  = rf.find("leftFrame").asString();
        leftFrame.src_frame_id  = rf.find("rootFrame").asString();
        rightFrame.dst_frame_id = rf.find("rightFrame").asString();
        rightFrame.src_frame_id = rf.find("rootFrame").asString();
        invertPOV = rf.find("invertPOV").asBool();

        if (param.parse(rf,  "limit", ParamParser::TYPE_DOUBLE))
        {
            double limitScalar = rf.find("limit").asDouble();
            limit[0] = limit[1] = limit[2] = limitScalar;
        }
        else
        {
            limit[0] = limit[1] = limit[2] = 1e12;
        }


        
        cfg.put("device", "JoypadControlClient");
        cfg.put("local", "/framecontroller/joypad-client");
        cfg.put("remote", rf.find("remote").asString());

        if (!joypadPD.open(cfg))
        {
            return false;
        }

        cfg.put("device", "transformClient");
        cfg.put("remote", "/transformServer");
        cfg.put("local", "/framecontroller/transform-client");
        
        if (!tfPD.open(cfg))
        {
            return false;
        }

        if (!joypadPD.view(joypad))
        {
            yError() << "unable to attach JoypadController interface to the PolyDriver object";
            return false;
        }
        
        if (!tfPD.view(tfPublisher))
        {
            yError() << "unable to attach JoypadController interface to the PolyDriver object";
            return false;
        }


        if (!joypad->getAxisCount(count) || count < axisCount || !joypad->getButtonCount(count) || count < buttonCount)
        {
            yError() << "joypad not compliant.. you need" << axisCount <<  "axes," << buttonCount << "button/s and " << stickCount << "stick/s";
            return false;
        }
        yarp::os::Time::delay(1);

        yInfo() << "waiting for left transform";
        if (!tfPublisher->waitForTransform(rf.find("leftFrameInitial").asString(), rf.find("rootFrame").asString(), 60.0))
        {
            yError() << "timeout waiting for transform";
            return false;
        };
        yInfo() << "left transform succesfully retrieved";

        yInfo() << "waiting for right transform";
        if (!tfPublisher->waitForTransform(rf.find("rightFrameInitial").asString(), rf.find("rootFrame").asString(), 60.0))
        {
            yError() << "timeout waiting for transform";
            return false;
        };
        yInfo() << "right transform succesfully retrieved";

        tfPublisher->getTransform(rf.find("leftFrameInitial").asString(), rf.find("rootFrame").asString(), leftInitM);
        tfPublisher->getTransform(rf.find("rightFrameInitial").asString(), rf.find("rootFrame").asString(), rightInitM);

        leftFrame.fromMatrix(leftInitM);
        rightFrame.fromMatrix(rightInitM);
        return true;
    }

    void deadband(double& v, double low)
    {
        if (fabs(v) < low)
        {
            v = 0;
        }
    }

    double hardlimiter(double v, double max, double min)
    {
        if (v < min)
        {
            return min;
        }

        if (v > max)
        {
            return max;
        }

        return v;
    }

    virtual bool   updateModule() override
    {
        size_t      i;
        static char assigned_to_left  = 1 - invertPOV;
        static char assigned_to_right = 0 + invertPOV;
        static char povDirection      = invertPOV * 2 - 1;
        for (i = 0; i < buttonCount; ++i)
        {
            joypad->getButton(i, buttonVector[i]);
        }

        if (buttonVector[7] > 0.5)
        {
            yInfo() << "home button pressed";
            leftFrame.fromMatrix(leftInitM);
            rightFrame.fromMatrix(rightInitM);
        }

        for (i = 0; i < axisCount; ++i)
        {
            joypad->getAxis(i, axisVector[i]);
            deadband(axisVector[i], 0.05);
            
            if (buttonVector[4 - i] > 0.1)
            {
                axisVector[i] = -axisVector[i];
            }
        }

        for (i = 0; i < stickCount; ++i)
        {
            joypad->getStick(i, sticks[i], IJoypadController::JypCtrlcoord_CARTESIAN);
            deadband(sticks[i][0], 0.05);
            deadband(sticks[i][1], 0.05);
            sticks[i][1] = -sticks[i][1];
        }

        yarp::sig::Vector lHome = leftInitM.subcol(0, 3, 3);
        yarp::sig::Vector rHome = rightInitM.subcol(0, 3, 3);

        leftFrame.translation.set( hardlimiter(leftFrame.translation.tX  + axisVector[assigned_to_left] * velocity, lHome[0] + limit[0], lHome[0] - limit[0]),
                                   hardlimiter(leftFrame.translation.tY  + povDirection * sticks[assigned_to_left][0] * velocity, lHome[1] + limit[1], lHome[1] - limit[1]),
                                   hardlimiter(leftFrame.translation.tZ  + sticks[assigned_to_left][1] * velocity, lHome[2] + limit[2], lHome[2] - limit[2]));
        rightFrame.translation.set( hardlimiter(rightFrame.translation.tX + axisVector[assigned_to_right] * velocity, rHome[0] + limit[0], rHome[0] - limit[0]),
                                    hardlimiter(rightFrame.translation.tY + povDirection * sticks[assigned_to_right][0] * velocity, rHome[1] + limit[1], rHome[1] - limit[1]),
                                    hardlimiter(rightFrame.translation.tZ + sticks[assigned_to_right][1] * velocity, rHome[2] + limit[2], rHome[2] - limit[2]));
        tfPublisher->setTransform(leftFrame.dst_frame_id, leftFrame.src_frame_id, leftFrame.toMatrix());
        tfPublisher->setTransform(rightFrame.dst_frame_id, rightFrame.src_frame_id, rightFrame.toMatrix());
        return true;
    }
};

int main(int argc, char *argv[])
{
    ResourceFinder& rf = ResourceFinder::getResourceFinderSingleton();
    rf.configure(argc, argv);

    FrameController fc;
    return fc.runModule(rf);
}

