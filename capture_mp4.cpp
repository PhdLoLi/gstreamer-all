#include <gst/gst.h>
#include <glib.h>
#include <string>
#include <iostream>
#include <unistd.h>

GstElement *pipeline;
static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
//  g_print ("bus_call here!\n");
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {
   case GST_MESSAGE_EOS:{
      g_printerr (("Got EOS from element \"%s\".\n"),
          GST_MESSAGE_SRC_NAME (msg));
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *err;
      gst_message_parse_error (msg, &err, &debug);
      g_printerr ("Debugging info: %s\n", (debug) ? debug : "none");
      g_free (debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

struct GstSample_Duo
{
  GstSample *video;
  GstSample *audio;
};

struct GstElement_Duo
{
  GstElement *video;
  GstElement *audio;
};

static void sig_int(int num)
{
//  GstPad *sinkpad = gst_element_get_static_pad (filesink, "sink");
  gst_element_send_event (pipeline, gst_event_new_eos());
//  gst_pad_send_event (sinkpad, gst_event_new_eos ());
//  gst_object_unref (sinkpad);
  return;
}

int 
main (int argc, 
char *argv[]) 
{ 
    GstElement *convert, *muxer, *demuxer, *mqueue, *filesink, *queue3; 
    GstElement_Duo source, queue, queue1, encoder, parser, sink, queue2, decoder;
    GMainLoop *loop;
    GstBus *bus;
  
    /* Initialisation */ 
    gst_init (NULL, NULL); 
    loop = g_main_loop_new (NULL, FALSE);
    signal(SIGINT, sig_int);

    /* Create gstreamer elements */ 
    pipeline = gst_pipeline_new ("capture-player"); 

//    source.video = gst_element_factory_make ("autovideosrc", "camera_source"); 
    source.video = gst_element_factory_make ("avfvideosrc", "camera_source"); 
    source.audio = gst_element_factory_make ("autoaudiosrc", "MIC_source"); 
//    source.audio = gst_element_factory_make ("osxaudiosrc", "MIC_source"); 

    convert = gst_element_factory_make ("audioconvert", "audio_convert"); 
    muxer = gst_element_factory_make ("mp4mux", "muxer"); 
    demuxer = gst_element_factory_make ("qtdemux", "demuxer"); 
    mqueue = gst_element_factory_make("multiqueue", "multi_queue");

    encoder.video = gst_element_factory_make("x264enc", "video_encoder");
    encoder.audio =  gst_element_factory_make("voaacenc", "audio_encoder");

    queue.video = gst_element_factory_make ("queue", "video_queue"); 
    queue.audio = gst_element_factory_make ("queue", "audio_queue"); 

    queue1.video = gst_element_factory_make ("queue", "video_queue1"); 
    queue1.audio = gst_element_factory_make ("queue", "audio_queue1"); 

    parser.video = gst_element_factory_make ("h264parse", "video_parser"); 
    parser.audio = gst_element_factory_make ("aacparse", "audio_parser"); 

    queue2.video = gst_element_factory_make ("queue", "video_queue2"); 
    queue2.audio = gst_element_factory_make ("queue", "audio_queue2"); 

    queue3 = gst_element_factory_make ("queue", "queue3"); 

    decoder.video = gst_element_factory_make ("avdec_h264", "video_decoder");
    decoder.audio = gst_element_factory_make ("faad", "audio_decoder"); 

//    sink.video = gst_element_factory_make ("appsink", "video_sink"); 
//    sink.audio = gst_element_factory_make ("appsink", "audio_sink"); 

    sink.video = gst_element_factory_make ("autovideosink", "video_sink"); 
    sink.audio = gst_element_factory_make ("autoaudiosink", "audio_sink"); 

    filesink = gst_element_factory_make("filesink", "filesink");

    if (!pipeline || !source.video || !source.audio || !convert || !encoder.video || !encoder.audio 
        || !queue1.video || !queue1.audio  || !parser.video || !parser.audio || !sink.audio || !sink.video) { 
      g_printerr ("One element could not be created. Exiting.\n"); 
    } 

    g_object_set (G_OBJECT (encoder.video), "speed-preset", 3, NULL); 
//    g_object_set (G_OBJECT (encoder.video), "byte-stream", TRUE, NULL); 
//    g_object_set (G_OBJECT (encoder.video), "qp-max", 30, NULL); 
//    g_object_set (G_OBJECT (encoder.video), "interlaced", TRUE, NULL); 

//    g_object_set (G_OBJECT (source.video), "do-timestamp", 1, NULL);
//    g_object_set (G_OBJECT (source.video), "num-buffers", -1, NULL);
//    g_object_set (G_OBJECT (source.video), "always-copy", FALSE, NULL);
//    g_object_set (G_OBJECT (source.audio), "num-buffers", -1, NULL);
//    g_object_set (G_OBJECT (source.audio), "do-timestamp", 1, NULL);
//    g_object_set (G_OBJECT (source.audio), "typefind", 1, NULL);
//
//    g_object_set (G_OBJECT (source.video), "sync", TRUE, NULL); 
//    g_object_set (G_OBJECT (source.audio), "sync", TRUE, NULL); 

//    g_object_set (G_OBJECT (sink.video), "sync", FALSE, NULL); 
//    g_object_set (G_OBJECT (sink.audio), "sync", FALSE, NULL); 

    /* Set up the pipeline */ 
    /* we set the input filename to the source element */ 
    /* we add all elements into the pipeline */ 
    gst_bin_add_many (GST_BIN (pipeline),source.video, queue1.video, encoder.video, queue2.video,
      source.audio, queue1.audio, convert, encoder.audio, queue2.audio, queue3,
      muxer,filesink, NULL); 

    gst_element_link_many (source.video, queue1.video, encoder.video, queue2.video, NULL); 
    gst_element_link_many (source.audio, queue1.audio, convert, queue3, encoder.audio, queue2.audio, NULL); 

    gst_element_link (muxer,filesink); 

    g_assert(gst_pad_link(gst_element_get_static_pad(queue2.video, "src"),gst_element_get_request_pad(muxer,"video_0")) == GST_PAD_LINK_OK); 
    g_assert(gst_pad_link(gst_element_get_static_pad(queue2.audio, "src"),gst_element_get_request_pad(muxer,"audio_0")) == GST_PAD_LINK_OK); 
//    g_assert(gst_pad_link(gst_element_get_static_pad(mqueue,"src_0"),gst_element_get_static_pad(parser.video, "sink")) == GST_PAD_LINK_OK); 
//    g_assert(gst_pad_link(gst_element_get_static_pad(mqueue,"src_1"),gst_element_get_static_pad(parser.audio, "sink")) == GST_PAD_LINK_OK); 
//    gst_element_link_many (queue.video, parser.video, decoder.video, sink.video, NULL); 
//    gst_element_link_many (queue.audio, parser.audio, decoder.audio, sink.audio, NULL); 
//    g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), &queue);

    g_object_set (G_OBJECT (filesink), "location", "/Users/lijing/next-ndnvideo/hehe.mp4" , NULL); 
    g_object_set (G_OBJECT (filesink), "sync", TRUE, NULL); 
    /* Set the pipeline to "playing" state*/ 
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, (GstBusFunc)bus_call, loop);
    gst_object_unref (bus);

    
    gst_element_set_state (pipeline, GST_STATE_PAUSED); 
    gst_element_set_state (pipeline, GST_STATE_PLAYING); 
    g_main_loop_run (loop);
 
//    sleep(2);
    gst_element_set_state (pipeline, GST_STATE_PAUSED); 
    gst_element_set_state (pipeline, GST_STATE_READY); 
    gst_element_set_state (pipeline, GST_STATE_NULL); 
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);

    g_print ("Deleting pipeline\n"); 
    return 0;

}
