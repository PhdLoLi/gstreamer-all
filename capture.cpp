#include <gst/gst.h>
#include <glib.h>
#include <string>
#include <iostream>
struct Parser
{
  GstElement *video;
  GstElement *audio;
};

static gboolean 
bus_call (GstBus *bus, 
GstMessage *msg, 
gpointer data) 

{ 
  GMainLoop *loop = (GMainLoop *) data; 
  switch (GST_MESSAGE_TYPE (msg)) { 
  case GST_MESSAGE_EOS: 
    g_print ("End of stream\n"); 
    g_main_loop_quit (loop); 
    break; 
  case GST_MESSAGE_ERROR: { 
    gchar *debug; 
    GError *error; 
    gst_message_parse_error (msg, &error, &debug); 
    g_free (debug); 
    g_printerr ("Error: %s\n", error->message); 
    g_error_free (error); 
    g_main_loop_quit (loop); 
    break; 
  } 
  default: 
  break; 
  } 
  return TRUE; 
} 

static void 
on_pad_added (GstElement *element, 
GstPad *pad, 
gpointer data) 
{ 
  GstPad *sinkpad; 
  GstCaps *caps;
  Parser *parser = (Parser *) data;
//  GstElement *parser = (GstElement *) data; 
  GstStructure *str;
  std::string type;
  /* We can now link this pad with the h264parse sink pad */ 
  caps =  gst_pad_get_current_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  type = gst_structure_get_name (str);

  g_print("%s\n", gst_caps_to_string(caps));

  if(type.find("video") != std::string::npos)
  {
    sinkpad = gst_element_get_static_pad (parser->video, "sink"); 
    gst_pad_link (pad, sinkpad); 
    gst_object_unref (sinkpad); 
    g_print ("linking demuxer/h264parse\n"); 
  }
  else
  {
    sinkpad = gst_element_get_static_pad (parser->audio, "sink"); 
    gst_pad_link (pad, sinkpad); 
    gst_object_unref (sinkpad); 
    g_print ("linking demuxer/accparse\n"); 
  }
} 

int 
main (int argc, 
char *argv[]) 
{ 
  GMainLoop *loop; 
  GstElement *pipeline, *source, *demuxer,*video_encoder, *video_parser, *video_decoder, *video_sink, * audio_decoder, * audio_sink; 
  Parser parser;  
  Parser queue;
  GstBus *bus; 
  /* Initialisation */ 
  gst_init (&argc, &argv); 
  loop = g_main_loop_new (NULL, FALSE); 
  /* Check input arguments */ 
  /* Create gstreamer elements */ 
  pipeline = gst_pipeline_new ("capture-player"); 
  source = gst_element_factory_make ("wrappercamerabinsrc", "camera-source"); 
//  demuxer = gst_element_factory_make ("qtdemux", "demuxer"); 
//  queue.video = gst_element_factory_make ("queue", "video_queue"); 
//  queue.audio = gst_element_factory_make ("queue", "audio_queue"); 
 // parser.video = gst_element_factory_make ("h264parse", "video_parser"); 
 // parser.audio = gst_element_factory_make ("aacparse", "audio_parser"); 
 // video_decoder = gst_element_factory_make ("avdec_h264", "video_decoder"); 
 // audio_decoder= gst_element_factory_make ("faad", "audio_decoder"); 
  video_encoder = gst_element_factory_make("x264enc", "video_encoder");
  video_parser = gst_element_factory_make("h264parse", "video_parser");
  video_decoder = gst_element_factory_make("avdec_h264", "video_decoder");
  video_sink = gst_element_factory_make ("autovideosink", "video_sinker"); 
//  audio_sink = gst_element_factory_make ("autoaudiosink", "audio_sink"); 
//  if (!pipeline || !source || !demuxer || !queue.video || !queue.audio  || !parser.video || !parser.audio || 
//      !video_decoder || !audio_decoder || !video_sink|| !audio_sink) { 
//    g_printerr ("One element could not be created. Exiting.\n"); 
//    return -1; 
//  } 
  /* Set up the pipeline */ 
  
  /* we add a message handler */ 
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 
  /* we add all elements into the pipeline */ 
//  gst_bin_add_many (GST_BIN (pipeline), source, demuxer, queue.audio, 
//  gst_bin_add_many (GST_BIN (pipeline), source, demuxer,queue.audio, queue.video, parser.video, video_decoder, video_sink, 
//      parser.audio, audio_decoder, audio_sink, NULL); 
  gst_bin_add_many (GST_BIN (pipeline), source, video_encoder, video_parser, video_decoder, video_sink, NULL); 
  /* we link the elements together */ 
//  gst_element_link_many (queue.audio, audio_sink, NULL); 
  gst_element_link_many (source, video_encoder, video_parser, video_decoder, video_sink, NULL); 
//  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added_queue), queue); 
//  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), &queue); 
  /* Set the pipeline to "playing" state*/ 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 
  /* Iterate */ 
  g_print ("Running...\n"); 
  g_main_loop_run (loop); 
  /* Out of the main loop, clean up nicely */ 
  g_print ("Returned, stopping playback\n"); 
  gst_element_set_state (pipeline, GST_STATE_NULL); 
  g_print ("Deleting pipeline\n"); 
  gst_object_unref (GST_OBJECT (pipeline)); 
  return 0; 
}
