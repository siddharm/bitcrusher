/*Bitcrusher using LADSPA*/
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "ladspa.h"
#define NPORTS 4

//defining ports
enum PortIndex {
  INPUT = 0,
  OUTPUT = 1,
  SAMPLE_RATE = 2,
  BIT_DEPTH = 3, 
};

//port settings
static const LADSPA_PortDescriptor
port_descriptors[NPORTS] = {
  [INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO,
  [OUTPUT]  = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
  [SAMPLE_RATE] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL,
  [BIT_DEPTH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL

};

//naming ports
static const char *
port_names[NPORTS] = {
  [INPUT] = "In",
  [OUTPUT]  = "Out",
  [SAMPLE_RATE] = "Frequency (kHz) - sample rate",
  [BIT_DEPTH] = "Resolution (bits) - bit depth"
};

//port input hints (useful for a UI interface)
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
    .LowerBound = 0.001,
    .UpperBound = 1
  },
  [BIT_DEPTH] = {
    .HintDescriptor = (LADSPA_HINT_BOUNDED_BELOW |
     LADSPA_HINT_BOUNDED_ABOVE |
     LADSPA_HINT_INTEGER		 |
     LADSPA_HINT_DEFAULT_MIDDLE),
    .LowerBound = 0,
    .UpperBound = 32
  },
};

//defining the data in each instance of a Bitcrusher
typedef struct {
  const LADSPA_Data *bit_depth;
  const LADSPA_Data *new_sample_rate;
  
  LADSPA_Data *out;
  LADSPA_Data *in;

  int count;
  LADSPA_Data last_out; 
  unsigned long sr;
} Bitcrusher;

//allocating space for an instance of a Bitcrusher
static LADSPA_Handle
instantiate(const LADSPA_Descriptor *descriptor,
  unsigned long sr)
{
  Bitcrusher* bc;

  bc = calloc(1, sizeof(Bitcrusher));
  if (!bc)
    return NULL;

  bc->count = 0.0f;
  bc->last_out = 0.0f;
  bc->sr = sr;

  return bc;
}

//freeing Bitcrusher space
static void
cleanup(LADSPA_Handle instance)
{
  free(instance);
}

//assigning the instance
static void
activate(LADSPA_Handle instance)
{
  Bitcrusher *bc = instance;
}

//connecting each of the ports
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
    bc->new_sample_rate = data;
    break;
  }
}

#define ROUND(f) ((float)((f > 0.0) ? floor(f + 0.5) : ceil(f - 0.5)))

//callback function that applies the effect to each input sample
static void
run(LADSPA_Handle instance, unsigned long nsamples)
{
  unsigned long i;
  Bitcrusher *bc = instance;

  const LADSPA_Data bit_depth = *(bc->bit_depth);
  const LADSPA_Data new_sample_rate = *(bc->new_sample_rate);
  LADSPA_Data *in = bc->in;
  LADSPA_Data *out = bc->out;

  float count = 0.0f;
  float last_out = 0.0f;
  float step, stepr, delta;

  double temp;

  step = pow(0.5f, (float)bit_depth - 0.999f);
  stepr = 1.0f/(float)bit_depth;

  float max = pow(2.0f, bit_depth) - 1;

  float ratio;
  if (new_sample_rate >= bc->sr) {
  	ratio = 1.0f;
  } else {
  	ratio = (float)new_sample_rate / (float)bc->sr;
  }


  for (i = 0; i < nsamples; i++) {
  count += ratio;

    //this loop keeps duplicates output values for the given frequency reduction rate, using count as a flag for when to switch
    if (count >= 1.0f) {
      count -= 1.0f;

      //here, we modify the bit depth
      //the value of the sample is transposed and rounded to fit into range

      //check if its in the negative half of the range
      float sign = 1.0f;
      if (in[i] < 0) {
        sign = -1.0f;
      }

      //take the integer part (analogous to rounding)
      delta = modf((in[i] + sign * step * 0.5f) * stepr, &temp) * step;
      last_out = in[i] - delta;
      out[i] = last_out;

    } else {
      out[i] = last_out;
    }
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

const LADSPA_Descriptor *
ladspa_descriptor(unsigned long index)
{
  return index == 0 ? &descriptor : NULL;
}
