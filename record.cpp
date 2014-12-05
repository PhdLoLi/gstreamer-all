#include <gst/gst.h>
#include <glib.h>
#include <string>
#include <iostream>
struct Parser
{
  GstElement *video;
  GstElement *audio;
};

static void
*produce_frame(void * threadData)
{
  GstCaps *caps;
  GstSample *sample;
  std::string streaminfo;
  GstBuffer *buffer;
  GstMapInfo map;
  GstElement* sink;
  sink = (GstElement *) threadData;

    size_t framenumber = 0;
    do {
      g_signal_emit_by_name (sink, "pull-sample", &sample);
      if (sample == NULL){
        g_print("Meet the EOS!\n");
        break;
        }
      if ( framenumber == 0)
      {
        caps = gst_sample_get_caps(sample);
        streaminfo = gst_caps_to_string(caps);
//        Name streaminfoVideoSuffix("video");
//        std::cout << streaminfo << std::endl;
//        producerStreaminfo->produce(streaminfoVideoSuffix, (uint8_t *)streaminfo.c_str(), streaminfo.size()+1);
        std::cout << "produce video streaminfo OK! " << streaminfo << std::endl;
        std::cout << "streaminfo size "<< streaminfo.size() + 1 << std::endl;
      }
      buffer = gst_sample_get_buffer (sample);
      gst_buffer_map (buffer, &map, GST_MAP_READ);
//      Name frameSuffix("video/" + std::to_string(framenumber));
      std::cout << "Frame number: "<< framenumber <<std::endl;
      std::cout << "Frame Size: "<< map.size * sizeof(uint8_t) <<std::endl;

//      producerFrame->produce(frameSuffix, (uint8_t *)map.data, map.size * sizeof(uint8_t));
      framenumber ++;
//      if ( framenumber > 2500)
//        break;
      if (sample)
        gst_sample_unref (sample);
      }while (sample != NULL);

    pthread_exit(NULL);
}

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
  GstElement *pipeline, *video_source, *demuxer,*video_encoder, *video_parser, *video_decoder, *video_sink, * audio_decoder, * audio_sink, *audio_source, *audio_encoder, *audio_convert, *audio_parser; 
  GstBus *bus; 

  /* Initialisation */ 
  gst_init (&argc, &argv); 
  loop = g_main_loop_new (NULL, FALSE); 
  /* Check input arguments */ 
  /* Create gstreamer elements */ 
  pipeline = gst_pipeline_new ("capture-player"); 
  video_source = gst_element_factory_make ("autovideosrc", "camera-source"); 
//  video_source = gst_element_factory_make ("wrappercamerabinsrc", "camera-source"); 
  video_encoder = gst_element_factory_make("x264enc", "video_encoder");
  video_parser = gst_element_factory_make("h264parse", "video_parser");
  video_decoder = gst_element_factory_make("avdec_h264", "video_decoder");
  video_sink = gst_element_factory_make ("appsink", "video_sinker"); 

  audio_source = gst_element_factory_make ("autoaudiosrc", "MIC-source"); 
  audio_convert = gst_element_factory_make ("audioconvert", "audio-convert"); 
  audio_encoder = gst_element_factory_make("voaacenc", "audio_encoder");
  audio_parser = gst_element_factory_make("aacparse", "audio_parser");
  audio_decoder= gst_element_factory_make("faad", "audio_decoder");
  audio_sink = gst_element_factory_make ("appsink", "audio_sinker"); 
//  if (!pipeline || !video_source || !demuxer || !queue.video || !queue.audio  || !parser.video || !parser.audio || 
//      !video_decoder || !audio_decoder || !video_sink|| !audio_sink) { 
//    g_printerr ("One element could not be created. Exiting.\n"); 
//    return -1; 
//  } 
  /* Set up the pipeline */ 
  
  g_object_set (G_OBJECT (video_sink), "sync", FALSE, NULL);
  g_object_set (G_OBJECT (audio_sink), "sync", FALSE, NULL);
  /* we add a message handler */ 
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 
  /* we add all elements into the pipeline */ 
  gst_bin_add_many (GST_BIN (pipeline), video_source, video_encoder, video_parser, video_sink, NULL); 
  /* we link the elements together */ 
  gst_element_link_many (video_source, video_encoder, video_parser, video_sink, NULL); 

  gst_bin_add_many (GST_BIN (pipeline), audio_source, audio_convert, audio_encoder, audio_parser, audio_sink, NULL); 
  /* we link the elements together */ 
  gst_element_link_many (audio_source, audio_convert, audio_encoder, audio_parser, audio_sink, NULL); 
  /* Set the pipeline to "playing" state*/ 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 

  pthread_t video_thread;
  int video_rc;
  video_rc = pthread_create(&video_thread, NULL, produce_frame, (void *)video_sink);

  pthread_t audio_thread;
  int audio_rc;
  audio_rc = pthread_create(&audio_thread, NULL, produce_frame, (void *)audio_sink);

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
