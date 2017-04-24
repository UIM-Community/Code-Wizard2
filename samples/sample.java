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

        do {
            probe = new NimProbe("example_probe","1.00","My company",args);

            NimConfig config = NimConfig.getInstance();
            int key2 = config.getValueAsInt("/config/java","key2",0);
            int log_level = config.getValueAsInt("/setup","loglevel",NimLog.WARN);
            interval = config.getValueAsInt("/setup","interval",interval);

            logger.log(0," - ---STARTING - ----");
            logger.setLogLevel(log_level);
            probe.setSubscribeSubject("alarm");
            probe.registerHubpostCallback(this,"hubpostCallback");
            probe.registerCallback(this,"test1","callbackTest1");
            probe.registerCallback(this,"test2","callbackTest2",new String[]{"id","text"});
            probe.registerCallbackOnTimer(this,"callbackOnTimer_one",interval*1000,false);
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
                System.out.println(NimUtility.displayPDS(pdss[entry],true));
            }

            PDS pds = new PDS();
            pds.put("name","Donald Duck");
            pds.put("phone","123-123-123");
            pds.put("age",60);
            NimPost post = new NimPost("YOUR_SUBJECT",pds);
            post.send();
        } while (probe.doForever());

        logger.log(NimLog.WARN," - ---STOPPING - ----");
    }

    public static void hubpostCallback(NimSession session, PDS pdsuserdata) throws NimException {
        session.sendReply(NimException.OK,null);
    }

    static public void callbackTest1 (NimSession session) throws NimException {
        String ts = "abcdefghijklmnopqrstuvwxyzeøå ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ 1234567890";
        PDS pds = new PDS();
        pds.put("ret_para",ts);
        session.sendReply(0,pds);
    }

    static public void callbackTest2(NimSession session, int id, String text) throws NimException {
        String ts = "abcdefghijklmnopqrstuvwxyzeøå ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ 1234567890";
        PDS pds = new PDS();
        pds.put("ret_id",id);
        pds.put("ret_text",text);
        pds.put("ret_para",ts);
        session.sendReply(0,pds);
    }

    public static void callbackOnTimer_one() throws NimException {
             try {
                 ConfigurationItem ConfItem = new ConfigurationItem("citype", "ciname","Hostname");
                 NimAlarm alarm = new NimAlarm(NimAlarm.NIML_WARNING, "Alarm generated from Java","2","JAVA","JAVA",ConfItem,"ciMetricId");
                 alarm.send();
                 alarm.disconnect();
             } catch (IOException e) {
                 e.printStackTrace();
              }
    }

    public static void callbackOnTimer_two() throws NimException {
            try {
 
            String target = "codewizard";
            int samplevalue = 12345;
            ConfigurationItem ConfItem = new ConfigurationItem("citype", "ciname","Hostname");
            NimQoS qos = new NimQoS(ConfItem,"metricId","QOS_TEST",false);
            qos.setDefinition("QOS_APPLICATION","My application response","milliseconds","ms"); 
            qos.setSampleRate(interval);
            qos.setTarget("codewizard");
            qos.setValue(12345);
            qos.send(); 

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
