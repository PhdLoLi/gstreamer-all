#include <gst/gst.h>
#include <glib.h>
#include <string>
#include <iostream>
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
  GstElement *parser = (GstElement *) data; 
  GstStructure *str;
  /* We can now link this pad with the h264parse sink pad */ 
  caps =  gst_pad_get_current_caps (pad);

  g_print ("Dynamic pad created, linking demuxer/parser\n"); 
  g_print("%s\n", gst_caps_to_string(caps));
  caps = gst_caps_make_writable(caps);
  str = gst_caps_get_structure (caps, 0);
//  gst_structure_remove_fields (str,"level", "profile", "height", "width", "framerate", "pixel-aspect-ratio", NULL);
  const char * lala = gst_caps_to_string (caps);
  GstCaps *ee = gst_caps_from_string(lala);
   
  std::cout << "ee   " << gst_caps_to_string (ee) << std::endl;
  sinkpad = gst_element_get_static_pad (parser, "sink"); 
  gst_pad_link (pad, sinkpad); 
  gst_object_unref (sinkpad); 
} 

static void 
on_pad_added_parser (GstElement *element, 
GstPad *pad, 
gpointer data) 
{ 
  GstPad *sinkpad; 
  GstCaps *caps;
  GstElement *decoder = (GstElement *) data; 
  /* We can now link this pad with the h264parse sink pad */ 
//  gst_pad_use_fixed_caps (pad);
//  caps =  gst_pad_get_current_caps (pad);
//
//  g_print ("Dynamic pad created, linking parser/decoder\n"); 
//  g_print("caps: %s\n", gst_caps_to_string(caps));
//  sinkpad = gst_element_get_static_pad (decoder, "sink"); 
//  gst_pad_link (pad, sinkpad); 
//  gst_object_unref (sinkpad); 
//  gst_element_link(element, decoder);
} 

int 
main (int argc, 
char *argv[]) 
{ 
  GMainLoop *loop; 
  GstElement *pipeline, *source, *demuxer, *parser, *decoder, *conv, *sink; 
  GstBus *bus; 
  /* Initialisation */ 
  gst_init (&argc, &argv); 
  loop = g_main_loop_new (NULL, FALSE); 
  /* Check input arguments */ 
  std::string filename;
  if (argc != 2) { 
    filename = "/Users/Loli/video/duoyan.mp4";
  }else
    filename = argv[1];
  /* Create gstreamer elements */ 
  pipeline = gst_pipeline_new ("mp4-player"); 
  source = gst_element_factory_make ("filesrc", "file-source"); 
  demuxer = gst_element_factory_make ("qtdemux", "demuxer"); 
  parser = gst_element_factory_make ("h264parse", "parser"); 
  decoder = gst_element_factory_make ("avdec_h264", "decoder"); 
  sink = gst_element_factory_make ("autovideosink", "video-output"); 
  if (!pipeline || !source || !demuxer || !parser || !decoder || !sink) { 
  g_printerr ("One element could not be created. Exiting.\n"); 
  return -1; 
  } 
  /* Set up the pipeline */ 
  /* we set the input filename to the source element */ 
  
  g_object_set (G_OBJECT (source), "location", filename.c_str(),  NULL); 
  /* we add a message handler */ 
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 
  /* we add all elements into the pipeline */ 
  gst_bin_add_many (GST_BIN (pipeline), 
  source, demuxer, parser, decoder, sink, NULL); 
  /* we link the elements together */ 
  gst_element_link (source, demuxer); 
  gst_element_link_many (parser, decoder, sink, NULL); 
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), parser); 
  g_signal_connect (parser, "pad-added", G_CALLBACK (on_pad_added_parser), decoder); 
  /* note that the demuxer will be linked to the decoder dynamically. 
  The reason is that Mp4 may contain various streams (for example 
  audio and video). The source pad(s) will be created at run time, 
  by the demuxer when it detects the amount and nature of streams. 
  Therefore we connect a callback function which will be executed 
  when the "pad-added" is emitted.*/ 
  /* Set the pipeline to "playing" state*/ 
  g_print ("Now playing: %s\n", filename.c_str()); 
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
