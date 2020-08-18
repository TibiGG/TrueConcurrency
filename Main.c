#include <stdio.h>
#include <string.h>

#include "Utils.h"
#include "Picture.h"
#include "PicProcess.h"

#define INPUT_PATH 1
#define OUTPUT_PATH 2
#define PROCESS 3
#define AUX 4

enum Process {
  INVERT,
  GRAYSCALE,
  ROTATE,
  FLIP,
  BLUR
};

struct parsed_args {
  char *input_path;
  char *output_path;
  enum Process proc;
  char *aux;
};

static void exit_if_bad_argc(int argc);
static void run_process(char *input_path, char *output_path, enum Process proc, char *aux);
static void process_picture(struct picture *pic, enum Process proc, char *aux);
static struct parsed_args get_arguments_from_user_input(int argc, char **argv);
static enum Process get_process(char *process_string);
static void add_aux_arg_if_necessary(struct parsed_args *arguments, char *const *argv, int argc);

int get_aux_as_int(char *aux);
int main(int argc, char **argv) {
  struct parsed_args args = get_arguments_from_user_input(argc, argv);
  run_process(args.input_path, args.output_path, args.proc, args.aux);
  return 0;
}

static struct parsed_args get_arguments_from_user_input(int argc, char **argv) {
  exit_if_bad_argc(argc);

  struct parsed_args arguments;
  arguments.input_path = argv[INPUT_PATH];
  arguments.output_path = argv[OUTPUT_PATH];
  arguments.proc = get_process(argv[PROCESS]);
  add_aux_arg_if_necessary(&arguments, argv, argc);

  return arguments;
}

static void add_aux_arg_if_necessary(struct parsed_args *arguments, char *const *argv, int argc) {
  if (arguments->proc == FLIP || arguments->proc == ROTATE) {
    if (argc == 5) {
      arguments->aux = argv[AUX];
    } else {
      printf("ERROR: not enough arguments. Ending session...\n");
      exit(-1);
    }
  } else if (argc == 5) {
    printf("ERROR: too many arguments. Ending session...\n");
    exit(-1);
  }
}

static void exit_if_bad_argc(int argc) {
  if (argc < 4) {
    printf("ERROR: not enough arguments. Ending session...\n");
    exit(-1);
  } else if (argc > 5) {
    printf("ERROR: too many arguments. Ending session...\n");
    exit(-1);
  }
}

static enum Process get_process(char *process_string) {
  if (strncmp(process_string, "invert", 8) == 0) {
    return INVERT;
  } else if (strncmp(process_string, "grayscale", 10) == 0) {
    return GRAYSCALE;
  } else if (strncmp(process_string, "rotate", 7) == 0) {
    return ROTATE;
  } else if (strncmp(process_string, "blur", 5) == 0) {
    return BLUR;
  } else if (strncmp(process_string, "flip", 5) == 0) {
    return FLIP;
  } else {
    printf("ERROR: this process does not exist. Abort...\n");
    exit(-1);
  }
}

static void run_process(char *input_path, char *output_path, enum Process proc, char *aux) {
  struct picture *pic = (struct picture *) malloc(sizeof(struct picture));

  init_picture_from_file(pic, input_path);
  process_picture(pic, proc, aux);
  save_picture_to_file(pic, output_path);

  free_picture(pic);
}

static void process_picture(struct picture *pic, enum Process proc, char *aux) {
  switch (proc) {
    case INVERT:invert_picture(pic);
      break;
    case GRAYSCALE:grayscale_picture(pic);
      break;
    case ROTATE:rotate_picture(pic, get_aux_as_int(aux));
      break;
    case FLIP:flip_picture(pic, *aux);
      break;
    case BLUR:blur_picture(pic);
      break;
    default:printf("ERROR: Unrecognised process. Abort...\n");
      exit(-1);
  }
}
int get_aux_as_int(char *aux) { return (int) strtol(aux, NULL, 10); }
