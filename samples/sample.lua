-----------------------------------------------------------------
-- CodeWizard:  NSA (LUA) 
-- This code was generated with the Nimsoft CodeWizard version 1.70
-- Date: lundi 24 avril 2017

local probename  = "example_probe"
local version    = "1.00"
local copyright  = "Copyright 2009, YourCompany"
local interval   = 300
local next_run   = 0
local loglevel   = 0
local sent_qosdf = false

-----------------------------------------------------------------
-- Message callback function
--
function hubpost (udata,header)
    local subject, level, message

    if header ~= nil then
        subject = header.subject
        probe.log (1,"(hubpost) - incoming message received - subject: %s",subject)
    end

    if udata ~= nil then
        level = udata.level
        message = udata.message

        if level ~= nil and message ~= nil then
           probe.log (1,"(hubpost) - level: %d, message: %s",level,message);
        end
    end
    return NIME_OK
end

-----------------------------------------------------------------
-- Command-set callback function(s), with parameter transfer
--
function do_something (args)
    local reply

    if args ~= nil then
        if args.number ~= nil then probe.log(0,"number = %d",args.number) end
        if args.string ~= nil then probe.log(0,"string = %s",args.string) end

        if args.number ~= nil then
            probe.log (0,"returning data in pds...")
            reply = pds.create()
            pds.putInt (reply,"status",1)
        end
    end
    return NIME_OK,reply
end

-----------------------------------------------------------------
function do_work()
    local now = timestamp.now()
    if now < next_run then return end
    next_run = now + interval

    probe.log (1,"(timeout) - interval has passed");
    ------------------------------
    -- Add your code here
    ------------------------------
end

-----------------------------------------------------------------
-- publishQoS (target, value) - Create and publish QoS message
--
function publishQoS (target,samplevalue)

    if not sent_qosdf then
        -- We only want to send the definition ONCE!
        probe.log (1,"Sending QoS definition.");
        nimbus.qos_definition ("QOS_TEST","QOS_APPLICATION","Test Application Response","Milliseconds","ms",false)
        sent_qosdf = true
    end

    nimbus.qos ("QOS_TEST",nil,target,samplevalue,interval) -- or swap interval with QOS_ASYNCH if non-interval. 
end

--
-- Open configuration file (probename + ".cfg") or specify it explicitly with probe.config("other.cfg")
--
function load_configuration()
    local cfg = probe.config()

    if cfg ~= nil and cfg["/setup"] then
        -- Look for the setup/your_key value
        local value = cfg["/setup"].your_key
        if cfg["/setup"].loglevel ~= nil then loglevel = cfg["/setup"].loglevel end
        probe.setloglevel (loglevel)
        if cfg["/setup"].interval ~= nil then interval = cfg["/setup"].interval end
    end
end

-----------------------------------------------------------------
-- Service functions
--
function restart ()
    probe.log(1,"(restart) - got restarted");
    load_configuration()
    probe.setloglevel(loglevel)
end

function timeout ()
    probe.log(2,"(timeout) - got kicked ")
    do_work()
end

-----------------------------------------------------------------
-- MAIN ENTRY
--

probe.register(probename,version,copyright)

load_configuration()

-- Simple alarm - severity level (1-5 or constant) and message string
nimbus.alarm (NIML_MAJOR,"Enter your simple message");

--Advanced alarm - severity level (1-5 or constant), message string, checkpoint-id/suppression-id, subsystem id
nimbus.alarm (NIML_MAJOR,"Enter your advanced message","probe.checkpoint.id","1.1.1")


--Create and publish a message using PDS 
local msg = pds.create()
pds.putString(msg,"name","Donald Duck")
pds.putInt   (msg,"age", 60)
nimbus.post  ("YOUR_SUBJECT",msg)
pds.delete   (msg)


-- Create and publish a Quality of Service message 
publishQoS ("example_probe",1234)     -- sends value 1234
publishQoS ("example_probe",nil)      -- sends value NULL


probe.addCallback ("do_something", "string,number%d");

--
-- Register a message subscriber (with or without a queue)
--
if probe.attach(probename) ~= NIME_OK then
    probe.log (0,"no queue defined, using subscribe")
    if probe.subscribe ("alarm") ~= NIME_OK then
        probe.log (0,"failed to subscribe on default hub")
        exit(1)
    end
end

probe.run ()

probe.unregister()
probe.log(0,"Exiting program");
