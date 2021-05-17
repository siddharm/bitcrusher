/*Bitcrusher using LADSPA*/
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "ladspa.h"
#define NPORTS 4





enum PortIndex {
  INPUT = 0,
  OUTPUT = 1,
  SAMPLE_RATE = 2,
  BIT_DEPTH = 3, 
};

static const LADSPA_PortDescriptor
port_descriptors[NPORTS] = {
  [INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO,
  [OUTPUT]  = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
  [SAMPLE_RATE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL,
  [BIT_DEPTH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL

};

static const char *
port_names[NPORTS] = {
  [INPUT] = "In",
  [OUTPUT]  = "Out",
  [SAMPLE_RATE] = "Frequency (kHz)",
  [BIT_DEPTH] = "Resolution (bits)"
};

static const LADSPA_PortRangeHint
port_hints[NPORTS] = {
  [INPUT] = {
    .HintDescriptor = 0
  },
  [OUTPUT] = {
    .HintDescriptor = 0
  },

  [SAMPLE_RATE] = {
    .HintDescriptor = (LADSPA_HINT_BOUNDED_BELOW |
                       LADSPA_HINT_BOUNDED_ABOVE |
                       LADSPA_HINT_SAMPLE_RATE   |
                       LADSPA_HINT_LOGARITHMIC   |
                       LADSPA_HINT_DEFAULT_MIDDLE),
    .LowerBound = 0,
    .UpperBound = 0.5
  },
  [BIT_DEPTH] = {
    .HintDescriptor = (LADSPA_HINT_BOUNDED_BELOW |
                       LADSPA_HINT_BOUNDED_ABOVE |
                       LADSPA_HINT_LOGARITHMIC   |
                       LADSPA_HINT_DEFAULT_MIDDLE),
    .LowerBound = 0,
    .UpperBound = 24
  },
};


typedef struct {
  const LADSPA_Data *bit_depth;
  const LADSPA_Data *sample_rate;
  
  LADSPA_Data *out;
  LADSPA_Data *in;

  unsigned long sr;
} Bitcrusher;

static LADSPA_Handle
instantiate(const LADSPA_Descriptor *descriptor,
            unsigned long sr)
{
  Bitcrusher* bc;

  bc = calloc(1, sizeof(Bitcrusher));
  if (!bc)
    return NULL;

  bc->sr = sr;

  return bc;
}

static void
cleanup(LADSPA_Handle instance)
{
  free(instance);
}


static void
activate(LADSPA_Handle instance)
{
  Bitcrusher *bc = instance;
}

static void
connect_port(LADSPA_Handle instance,
             unsigned long port, LADSPA_Data *data)
{
  Bitcrusher *bc = instance;

  switch (port) {
  case INPUT:
    bc->in = data;
    break;
  case OUTPUT:
    bc->out = data;
    break;
  case BIT_DEPTH:
    bc->bit_depth = data;
    break;
  case SAMPLE_RATE:
    bc->sample_rate = data;
    break;
  }
}

static void
run(LADSPA_Handle instance, unsigned long nsamples)
{
  unsigned long i;
  Bitcrusher *bc = instance;

  const LADSPA_Data bit_depth = *(bc->bit_depth);
  LADSPA_Data *in = bc->in;
  LADSPA_Data *out = bc->out;
  double sr = bc->sr;

  for (i = 0; i < nsamples; i++) {
    out[i] = in[i];
  }

}



static const LADSPA_Descriptor
descriptor = {
  .UniqueID = 1111,
  .Label = "bitcrusher",
  .Name = "bitcrusher",
  .Maker = "Siddha Mavuram",
  .Copyright = "unlicensed",
  .Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE,

  .PortCount = NPORTS,
  .PortDescriptors = port_descriptors,
  .PortNames = port_names,
  .PortRangeHints = port_hints,

  .instantiate = instantiate,
  .connect_port = connect_port,
  .activate = activate,
  .run = run,
  .cleanup = cleanup
};

//LADSPA_SYMBOL_EXPORT;
const LADSPA_Descriptor *
ladspa_descriptor(unsigned long index)
{
  return index == 0 ? &descriptor : NULL;
}
