//#################################################################
//# CodeWizard:  Java
//# This code was generated with the Nimsoft CodeWizard version 1.70
//# Date: lundi 24 avril 2017
//
package example;
import java.io.IOException;
import com.nimsoft.nimbus.*;
import com.nimsoft.nimbus.ci.ConfigurationItem;

public class example_probe {
    static NimLog logger = NimLog.getLogger(example_probe.class);
    static int interval = 300;

    public static void main(String[] args) {
        try {
            example_probe exampleprobe = new example_probe();
            exampleprobe.doit(args);
        }catch(Exception e) {
            e.printStackTrace();
        }
    }

    public void doit(String[] args) throws Exception {
        NimProbe  probe = null;

        // Loop to handle restart (_restart) command
        // Stop (_stop) command terminates the loop
        do {

            // Construct the probe
            probe = new NimProbe("example_probe","1.00","My company",args);

            NimConfig config = NimConfig.getInstance();
            int key2 = config.getValueAsInt("/config/java","key2",0);
            int log_level = config.getValueAsInt("/setup","loglevel",NimLog.WARN);
            interval = config.getValueAsInt("/setup","interval",interval);

            logger.log(0," - ---STARTING - ----");
            logger.setLogLevel(log_level);

            // Subscribe to message from the hub, register the "hubpost" callback method
            probe.setSubscribeSubject("alarm");
            probe.registerHubpostCallback(this,"hubpostCallback");

            // Subscribe to the command test1 with no parameters
            probe.registerCallback(this,"test1","callbackTest1");
            // Subscribe to the command test2 with two parameters
            probe.registerCallback(this,"test2","callbackTest2",new String[]{"id","text"});

            // Register timer callbacks called on timer and executes in the main thread
            probe.registerCallbackOnTimer(this,"callbackOnTimer_one",interval*1000,false);
            // Register timer callbacks called on timer and executes in a separate thread (not the main thread)
            probe.registerCallbackOnTimer(this,"callbackOnTimer_two",interval*1000,true);

            NimRequest request = new NimRequest("controller","gethub");
            PDS pdsout = request.send();

            String hubaddress =
                "/" + pdsout.getString("hubdomain") +
                "/" + pdsout.getString("hubname") +
                "/" + pdsout.getString("hubrobotname") +
                "/hub";

            request = new NimRequest(hubaddress,"getrobots");
            pdsout = request.send();

            PDS[] pdss = pdsout.getTablePDSs("robotlist");
            for (int entry=0;entry<pdss.length;entry++) {
                // This example only prints all the PDSs content to the console
                System.out.println(NimUtility.displayPDS(pdss[entry],true));
            }

            // Create a PDS using the PDS class.
            // Add two strings and a number to the PDS.
            // Post (send) it
            PDS pds = new PDS();
            pds.put("name","Donald Duck");
            pds.put("phone","123-123-123");
            pds.put("age",60);
            NimPost post = new NimPost("YOUR_SUBJECT",pds);
            post.send();

            // Run until a stop signal is received from the manager.
            // Restart initiates re-creation of the probe.
            // The doForever() method returns true or false
            //   true  - a restart command (_restart) is received
            //   false - a stop command (_stop) is received
        } while (probe.doForever());

        logger.log(NimLog.WARN," - ---STOPPING - ----");
    }

    /**
     * The "hubpost" callback that receives a PDS with some userdata
     * @param session The callback session
     * @param pdsuserdata The PDS user data
     */
    public static void hubpostCallback(NimSession session, PDS pdsuserdata) throws NimException {
        session.sendReply(NimException.OK,null); // OK return
    }

    /**
     * The "test1" callback command that has no parameters
     * The method returns a text string
     * @param session The callback session
     */
    static public void callbackTest1 (NimSession session) throws NimException {
        String ts = "abcdefghijklmnopqrstuvwxyzeøå ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ 1234567890";
        PDS pds = new PDS();
        pds.put("ret_para",ts);
        session.sendReply(0,pds);
    }

    /**
     * The "test2" callback command that has two parameters, an id and some text
     * The method returns the received parameters + an additional "full alpabeth" text string
     * @param session The callback session
     * @param id An id
     * @param text Some text
     */
    static public void callbackTest2(NimSession session, int id, String text) throws NimException {
        String ts = "abcdefghijklmnopqrstuvwxyzeøå ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ 1234567890";
        PDS pds = new PDS();
        pds.put("ret_id",id);
        pds.put("ret_text",text);
        pds.put("ret_para",ts);
        session.sendReply(0,pds);
    }

    /**
     * This method is called as defined in the register method
     */
    public static void callbackOnTimer_one() throws NimException {
        // Do something, like testing an sending alarm, Qos,...
            // Define and send a warning alarm
             try {
                 ConfigurationItem ConfItem = new ConfigurationItem("citype", "ciname","Hostname");
                 NimAlarm alarm = new NimAlarm(NimAlarm.NIML_WARNING, "Alarm generated from Java","2","JAVA","JAVA",ConfItem,"ciMetricId");
                 alarm.send();
                 alarm.disconnect();
             } catch (IOException e) {
                 e.printStackTrace();
              }
    }

    /**
     * This method is called as defined in the register method
     */
    public static void callbackOnTimer_two() throws NimException {
        // Do something else
            try {
            // Create a QoS object
            // Set an optional defintion
            // Set some values
            // Send
            String target = "codewizard";
            int samplevalue = 12345;
            ConfigurationItem ConfItem = new ConfigurationItem("citype", "ciname","Hostname");
            NimQoS qos = new NimQoS(ConfItem,"metricId","QOS_TEST",false);
            qos.setDefinition("QOS_APPLICATION","My application response","milliseconds","ms"); // Optional
            qos.setSampleRate(interval);
            qos.setTarget("codewizard");
            qos.setValue(12345);
            qos.send(); // The definition, if defined will be sent first, but only once

            for (int i=0;i<20;i++) {
                qos.setValue(((i % 3)==0) ? 0 : i);
                qos.send();
            }
            qos.close();
            } catch (Exception e) {
                 e.printStackTrace();
            }
    }

}
