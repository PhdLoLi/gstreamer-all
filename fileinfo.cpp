#include <glib.h>
#include "tools.hpp"
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

  struct VideoAudio 
  {
    GstElement *video_parser;
    std::string video_info;
    GstElement *audio_parser;
    std::string audio_info;
  } ; 
static void 
on_pad_added(GstElement *element, 
GstPad *pad, 
gpointer data) 
{ 
  GstPad *sinkpad; 
  GstCaps *caps;
  const GstStructure *str;
  std::string type;
  struct VideoAudio * video_audio = (struct VideoAudio *) data;

  caps =  gst_pad_get_current_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  type = gst_structure_get_name (str);

  std::cout << "TYPE: " << type << std::endl;
  if(type.find("video") != std::string::npos)
  {
    video_audio->video_info = gst_caps_to_string(caps);
    /* We can now link this pad with the h264parse sink pad */ 
    g_print ("Dynamic pad created, linking demuxer/video_parser\n"); 
    std::cout << "Caps inside on_pad_added: " << video_audio->video_info <<std::endl;
    sinkpad = gst_element_get_static_pad (video_audio->video_parser, "sink"); 
    gst_pad_link (pad, sinkpad); 
    gst_object_unref (sinkpad); 
  }
  return; 
} 

int 
main (int argc, 
char *argv[]) 
{ 
  struct VideoAudio  video_audio; 

  GstElement *pipeline, *source, *demuxer, *sink; 
  GstCaps *caps;
    
  GstBuffer *buffer;
  GstSample *sample;
  GstMapInfo map;

  /* Initialisation */ 
  gst_init (&argc, &argv); 
  /* Check input arguments */ 
  if (argc != 2) { 
    g_printerr ("Usage: %s <MP4 filename>\n", argv[0]); 
    return -1; 
  } 
  /* Create gstreamer elements */ 
  pipeline = gst_pipeline_new ("mp4-player"); 
  source = gst_element_factory_make ("filesrc", "file-source"); 
  demuxer = gst_element_factory_make ("qtdemux", "demuxer"); 
  video_audio.video_parser = gst_element_factory_make ("h264parse", "parser"); 
  sink = gst_element_factory_make ("appsink", NULL);

  g_object_set (G_OBJECT (sink), "sync", FALSE, NULL); 

  if (!pipeline || !source || !demuxer || !video_audio.video_parser || !sink) { 
    g_printerr ("One element could not be created. Exiting.\n"); 
    return -1; 
  } 
  /* Set up the pipeline */ 
  /* we set the input filename to the source element */ 
  g_object_set (G_OBJECT (source), "location", argv[1], NULL); 
  /* we add a message handler */ 
  /* we add all elements into the pipeline */ 
  gst_bin_add_many (GST_BIN (pipeline), source, demuxer, video_audio.video_parser, sink, NULL); 
  /* we link the elements together */ 
  gst_element_link (source, demuxer); 
  gst_element_link_many (video_audio.video_parser, sink, NULL); 
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), &video_audio); 
  /* Set the pipeline to "playing" state*/ 
  g_print ("Now playing: %s\n", argv[1]); 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 
  
  g_signal_emit_by_name (sink, "pull-sample", &sample);
  caps = gst_sample_get_caps(sample);
//  Tools::read_video_props(caps);
  g_print("caps: %s\n", gst_caps_to_string(caps));
  std::cout << "video_info " << video_audio.video_info << std::endl;
  
  if (sample)
    gst_sample_unref (sample);

  /* Iterate */ 
  gst_element_set_state (pipeline, GST_STATE_NULL); 
  gst_object_unref (GST_OBJECT (pipeline)); 
  return 0; 
}
