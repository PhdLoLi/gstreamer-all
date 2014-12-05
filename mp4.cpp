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

static void 
on_pad_added (GstElement *element, 
GstPad *pad, 
gpointer data) 
{ 
  GstPad *sinkpad; 
  GstElement *parser = (GstElement *) data; 
  /* We can now link this pad with the h264parse sink pad */ 
  g_print ("Dynamic pad created, linking demuxer/parser\n"); 
  sinkpad = gst_element_get_static_pad (parser, "sink"); 
  gst_pad_link (pad, sinkpad); 
  gst_object_unref (sinkpad); 
} 

int 
main (int argc, 
char *argv[]) 
{ 
  GMainLoop *loop; 
  GstElement *pipeline, *source, *demuxer, *parser, *sink; 
  GstBus *bus; 
  GstCaps *caps;
    
  GstBuffer *buffer;
  GstSample *sample;
  GstMapInfo map;

  /* Initialisation */ 
  gst_init (&argc, &argv); 
  loop = g_main_loop_new (NULL, FALSE); 
  /* Check input arguments */ 
  if (argc != 2) { 
    g_printerr ("Usage: %s <MP4 filename>\n", argv[0]); 
    return -1; 
  } 
  /* Create gstreamer elements */ 
  pipeline = gst_pipeline_new ("mp4-player"); 
  source = gst_element_factory_make ("filesrc", "file-source"); 
  demuxer = gst_element_factory_make ("qtdemux", "demuxer"); 
  parser = gst_element_factory_make ("h264parse", "parser"); 
  sink = gst_element_factory_make ("appsink", NULL);

  g_object_set (G_OBJECT (sink), "sync", FALSE, NULL); 

  if (!pipeline || !source || !demuxer || !parser || !sink) { 
    g_printerr ("One element could not be created. Exiting.\n"); 
    return -1; 
  } 
  /* Set up the pipeline */ 
  /* we set the input filename to the source element */ 
  g_object_set (G_OBJECT (source), "location", argv[1], NULL); 
  /* we add a message handler */ 
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 
  /* we add all elements into the pipeline */ 
  gst_bin_add_many (GST_BIN (pipeline), 
  source, demuxer, parser, sink, NULL); 
  /* we link the elements together */ 
  gst_element_link (source, demuxer); 
  gst_element_link_many (parser, sink, NULL); 
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), parser); 
  /* Set the pipeline to "playing" state*/ 
  g_print ("Now playing: %s\n", argv[1]); 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 
  time_t time_start = std::time(0);
  int framenumber = 0;
  
  do {
    g_signal_emit_by_name (sink, "pull-sample", &sample);
//    g_print("After calling pull-sample  ");
    if (sample == NULL){
      g_print("Meet the EOS!\n");
      break;
      }
    caps = gst_sample_get_caps(sample);
    Tools::read_video_props(caps);
    buffer = gst_sample_get_buffer (sample);
//    info = gst_sample_get_info(sample);
//    segment = gst_sample_get_segment(sample);
    gst_buffer_map (buffer, &map, GST_MAP_READ);
//    Name frameSuffix(std::to_string(framenumber));
    std::cout << "Frame number: "<< framenumber <<std::endl;
//    producer->produce(frameSuffix, (uint8_t *)map.data, map.size);
    std::cout << "Map Size: "<< map.size <<std::endl;
    framenumber ++;
  
    if (sample)
      gst_sample_unref (sample);
//    }while (framenumber <= 60*10*25);
    }while (sample != NULL);

  time_t time_end = std::time(0);
  double seconds = difftime(time_end, time_start);
  std::cout << seconds << " seconds have passed" << std::endl;
  
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
