#include <gst/gst.h>
#include <csignal>

static GMainLoop *loop = nullptr;

static void on_sigint(int /*signum*/) {
    if (loop) {
        g_main_loop_quit(loop);
    }
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);
    std::signal(SIGINT, on_sigint);

   auto* pipeline = gst_pipeline_new("pipeline");
   auto* videotestsrc = gst_element_factory_make("videotestsrc", "source");
   auto* videoconvert = gst_element_factory_make("videoconvert", "convert");
   auto* autovideosink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !videotestsrc || !videoconvert || !autovideosink) {
        g_printerr("Failed to create one or more elements.\n");
        if (pipeline) gst_object_unref(pipeline);
        if (videotestsrc) gst_object_unref(videotestsrc);
        if (videoconvert) gst_object_unref(videoconvert);
        if (autovideosink) gst_object_unref(autovideosink);
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipeline), videotestsrc, videoconvert, autovideosink, nullptr);

    if (!gst_element_link_many(videotestsrc, videoconvert, autovideosink, nullptr)) {
        g_printerr("Failed to link elements.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    auto ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    loop = g_main_loop_new(nullptr, FALSE);
    if (!loop) {
        g_printerr("Failed to create main loop.\n");
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        return -1;
    }


    g_print("Pipeline running. Press Ctrl+C to stop.\n");
    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);
    loop = nullptr;

    return 0;
}