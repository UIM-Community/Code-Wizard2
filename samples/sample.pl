#!/opt/nimbus/bin/perl
#################################################################
# CodeWizard:  perl 
# This code was generated with the Nimsoft CodeWizard version 1.70
# Date: lundi 24 avril 2017

use strict;
use Getopt::Std;
use Nimbus::API;
use Nimbus::Session;
use Nimbus::CFG;
use Nimbus::PDS;

my $prgname       = "example_probe";
my $sess;
my $version       = "1.00";
my $config;
my $int_option    = 0;
my $str_option    = "";
my %options;
my $loglevel      = 0;
my $logfile       = "$prgname.log";
my $next_run      = 0;
my $interval      = 300;
my $qosDefinition = 0;

###########################################################
sub hubpost {
    my $hMsg  = shift;     # Message handle
    my $udata = shift;     # User data/message
    my $full  = shift;     # Full message (inkl. header)

    my $subject = pdsGet_PCH($full,"subject");
    nimLog (1,"(hubpost) - incoming message received - subject: $subject");

    my $string = pdsGet_PCH ($udata,"message");
    my $number = pdsGet_INT ($udata,"level");
    nimLog (1,"(hubpost) - level: $number, message: $string");
    nimSendReply($hMsg);
}

###########################################################
# Command-set callback function(s), with parameter transfer
#
sub do_something {
    my ($hMsg,$arg1,$arg2,$arg3) = @_;
    my $reply = pdsCreate();
    nimLog (1,"(do_something) - serving client with arguments: $arg1,$arg2,$arg3");
    pdsPut_INT($reply,"status",1);
    nimSendReply($hMsg,0,$reply);
    pdsDelete($reply);
}

###########################################################
# DoWork - function called by dispatcher on timeout
#
sub doWork {
    my $now = time();
    return if ($now < $next_run);
    $next_run = $now + $interval;
    nimLog (1,"(doWork) - interval has passed");

    ###############################
    # Enter your code here
    ###############################
}

###########################################################
# publishQoS (target, value) - Create and publish QoS message
#
sub publishQoS {
    my $target      = shift;
    my $samplevalue = shift;
    my $source      = nimGetVarStr(NIMV_ROBOTNAME);
    my $ival        = $interval;                # Seconds

    if (!$qosDefinition) {
        #We only want to send the definition ONCE!
        nimQoSSendDefinition ("QOS_TEST",       # QOS Name
                 "QOS_APPLICATION",             # QOS Group
                 "Test Application Response",   # QOS Description
                 "Milliseconds","ms");          # QOS Unit and Abbreviation
        $qosDefinition = 1;
    }
    if (my $qos = nimQoSCreate("QOS_TEST",$source,$ival,-1)) {
        if ($samplevalue < 0) {
            nimQoSSendNull ($qos,$target);
        }else {
            nimQoSSendValue($qos,$target,$samplevalue);
        }
        nimQoSFree($qos);
    }
}

#######################################################################
# Service functions
#
sub restart {
    nimLog(1,"(restart) - got restarted");
    $config = Nimbus::CFG->new("$prgname.cfg");
    $int_option = $config->{setup}->{int_option} || 0;
    $str_option = $config->{setup}->{str_option} || "default";
    $loglevel = $options{d} || $config->{setup}->{loglevel}|| 0;
    $logfile  = $options{l} || $config->{setup}->{logfile} || "$prgname.log";
    $interval  = $options{i} || $config->{setup}->{interval}|| 300;
    nimLogSet($logfile,$prgname,$loglevel,0);
}

sub timeout {
    nimLog(2,"(timeout) - got kicked ");
    doWork();
}

###########################################################
# Signal handler - Ctrl-Break
#
sub ctrlc {
    exit;
}


###########################################################
# MAIN ENTRY
#
getopts("d:l:i:",\%options);

$SIG{INT} = \&ctrlc;

$config     = Nimbus::CFG->new("$prgname.cfg");
$int_option = $config->{setup}->{int_option} || 0;
$str_option = $config->{setup}->{str_option} || "default";
$loglevel   = $options{d} || $config->{setup}->{loglevel}|| 0;
$logfile    = $options{l} || $config->{setup}->{logfile} || "$prgname.log";
$interval   = $options{i} || $config->{setup}->{interval}|| 300;

nimLogSet($logfile,$prgname,$loglevel,0);
nimLog(0,"----------------- Starting  (pid: $$) ------------------");

#Simple alarm - severity level (1-5 or constant) and message string
nimAlarm(NIML_MAJOR,"Enter your simple message");

#Advanced alarm - severity level (1-5 or constant), message string, subsystem id, checkpoint-id
my $checkid = nimSuppToStr(0,0,0,"$prgname/xxx");
nimAlarm(NIML_MAJOR,"Enter your advanced message","1.1.1",$checkid);


#Create and publish a message using PDS 
my $pds = Nimbus::PDS->new();

$pds->string("name","Donald Duck");
$pds->number("age",60);
$pds->post ("YOUR_SUBJECT");


#Create and publish a Quality of Service message 
publishQoS("$prgname",1234);


$sess = Nimbus::Session->new("$prgname");
$sess->setInfo($version,"Nimsoft Corporation");
if ($sess->attach("$prgname")) {
    nimLog(1,"no queue defined, using subscribe");
    if ($sess->subscribe("alarm")) {
        nimLog(0,"failed to subscribe on default hub");
        exit;
    }
}


if ($sess->server (NIMPORT_ANY,\&timeout,\&restart)==0) {
    nimLog(1,"server session is created");
    $sess->addCallback ("do_something", "arg1,arg2_str,arg3_num%d");
}else {
    nimLog(0,"unable to create server session");
    exit(1);
}

$sess->dispatch();
nimLog(0,"Received STOP, terminating program");
nimLog(0,"Exiting program");
exit;
