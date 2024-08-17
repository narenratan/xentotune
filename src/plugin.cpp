#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "clap/include/clap/clap.h"
#include "signalsmith-stretch/signalsmith-stretch.h"
#include "MTS-ESP/Client/libMTSClient.h"

struct MyPlugin
{
    clap_plugin_t plugin;
    const clap_host_t *host;
    float sampleRate;
    signalsmith::stretch::SignalsmithStretch<float> stretch;
    MTSClient *client;
};

/**
 *  Map frequency onto nearest MTS-ESP frequency
 **/
float mapFreq(float inputFreq, float sampleRate, MTSClient *client)
{
    double iFreq, jFreq, mFreq;
    int i = 0;
    int j = 127;
    int m = 0;

    // inputFreq comes in normalised to sample rate
    iFreq = MTS_NoteToFrequency(client, i, 0) / sampleRate;
    jFreq = MTS_NoteToFrequency(client, j, 0) / sampleRate;

    // Return unmodified frequency if it lies outside MTS-ESP frequencies
    if ((inputFreq < iFreq) || (jFreq < inputFreq))
    {
        return inputFreq;
    }

    // Bisect MTS-ESP frequencies to find the two which inputFreq lies between
    while ((j - i) > 1)
    {
        m = (i + j) / 2;
        mFreq = MTS_NoteToFrequency(client, m, 0) / sampleRate;
        if (mFreq > inputFreq)
        {
            j = m;
        }
        else
        {
            i = m;
        };
    }

    iFreq = MTS_NoteToFrequency(client, i, 0) / sampleRate;
    jFreq = MTS_NoteToFrequency(client, j, 0) / sampleRate;

    // Return whichever frequency inputFreq is closest to in log terms (cents).
    // Returned frequency is also normalised to sample rate
    return ((log2(inputFreq) - log2(iFreq)) < (log2(jFreq) - log2(inputFreq))) ? iFreq : jFreq;
}

const char *_features[] = {
    CLAP_PLUGIN_FEATURE_STEREO,
    NULL,
};

static const clap_plugin_descriptor_t pluginDescriptor = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "naren.Xentotune",
    .name = "Xentotune",
    .vendor = "naren",
    .url = "https://github.com/narenratan/xentotune",
    .manual_url = "https://github.com/narenratan/xentotune",
    .support_url = "https://github.com/narenratan/xentotune",
    .version = "0.0.0",
    .description = "Microtonal autotuner",
    .features = _features,
};

static const clap_plugin_audio_ports_t extensionAudioPorts = {
    .count = [](const clap_plugin_t *plugin, bool isInput) -> uint32_t { return 1; },

    .get = [](const clap_plugin_t *plugin, uint32_t index, bool isInput,
              clap_audio_port_info_t *info) -> bool {
        if (index)
            return false;
        info->id = 0;
        info->channel_count = 2;
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = CLAP_INVALID_ID;
        snprintf(info->name, sizeof(info->name), "%s", isInput ? "Audio Input" : "Audio Output");
        return true;
    },
};

static const clap_plugin_t pluginClass = {
    .desc = &pluginDescriptor,
    .plugin_data = nullptr,

    .init = [](const clap_plugin *_plugin) -> bool {
        MyPlugin *plugin = (MyPlugin *)_plugin->plugin_data;
        (void)plugin;
        return true;
    },

    .destroy =
        [](const clap_plugin *_plugin) {
            MyPlugin *plugin = (MyPlugin *)_plugin->plugin_data;
            if (plugin->client)
            {
                MTS_DeregisterClient(plugin->client);
            }
            free(plugin);
        },

    .activate = [](const clap_plugin *_plugin, double sampleRate, uint32_t minimumFramesCount,
                   uint32_t maximumFramesCount) -> bool {
        MyPlugin *plugin = (MyPlugin *)_plugin->plugin_data;

        plugin->client = MTS_RegisterClient();

        plugin->sampleRate = sampleRate;

        plugin->stretch.presetDefault(2, sampleRate);
        MTSClient *client = plugin->client;
        plugin->stretch.setFreqMap([sampleRate, client](float inputFreq) {
            return mapFreq(inputFreq, sampleRate, client);
        });

        return true;
    },

    .deactivate = [](const clap_plugin *_plugin) {},

    .start_processing = [](const clap_plugin *_plugin) -> bool { return true; },

    .stop_processing = [](const clap_plugin *_plugin) {},

    .reset =
        [](const clap_plugin *_plugin) { MyPlugin *plugin = (MyPlugin *)_plugin->plugin_data; },

    .process = [](const clap_plugin *_plugin,
                  const clap_process_t *process) -> clap_process_status {
        MyPlugin *plugin = (MyPlugin *)_plugin->plugin_data;

        assert(process->audio_outputs_count == 1);
        assert(process->audio_inputs_count == 1);

        const uint32_t frameCount = process->frames_count;

        plugin->stretch.process(process->audio_inputs[0].data32, frameCount,
                                process->audio_outputs[0].data32, frameCount);

        return CLAP_PROCESS_CONTINUE;
    },

    .get_extension = [](const clap_plugin *plugin, const char *id) -> const void * {
        if (0 == strcmp(id, CLAP_EXT_AUDIO_PORTS))
            return &extensionAudioPorts;
        return nullptr;
    },

    .on_main_thread = [](const clap_plugin *_plugin) {},
};

static const clap_plugin_factory_t pluginFactory = {
    .get_plugin_count = [](const clap_plugin_factory *factory) -> uint32_t { return 1; },

    .get_plugin_descriptor = [](const clap_plugin_factory *factory, uint32_t index)
        -> const clap_plugin_descriptor_t * { return index == 0 ? &pluginDescriptor : nullptr; },

    .create_plugin = [](const clap_plugin_factory *factory, const clap_host_t *host,
                        const char *pluginID) -> const clap_plugin_t * {
        if (!clap_version_is_compatible(host->clap_version) ||
            strcmp(pluginID, pluginDescriptor.id))
        {
            return nullptr;
        }

        MyPlugin *plugin = (MyPlugin *)calloc(1, sizeof(MyPlugin));
        plugin->host = host;
        plugin->plugin = pluginClass;
        plugin->plugin.plugin_data = plugin;
        return &plugin->plugin;
    },
};

extern "C" const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,

    .init = [](const char *path) -> bool { return true; },

    .deinit = []() {},

    .get_factory = [](const char *factoryID) -> const void * {
        return strcmp(factoryID, CLAP_PLUGIN_FACTORY_ID) ? nullptr : &pluginFactory;
    },
};
