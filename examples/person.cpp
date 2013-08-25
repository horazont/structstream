#include <iostream>
#include <fstream>
#include <cstdint>

#include "structstream/structstream.hpp"

using namespace StructStream;

/* We will be reading and writing this structure to a structstream. */
struct PersonRecord {
    std::string name;
    uint32_t age;
};

/* Here we define a class which will handle deserialization of our
 * struct. */
typedef only<struct_decl<
    Container,
    id_selector<0x01>,
    struct_members<
        member<UTF8Record, id_selector<0x02>, PersonRecord, std::string, &PersonRecord::name>,
        member<UInt32Record, id_selector<0x03>, PersonRecord, uint32_t, &PersonRecord::age>
        >
    >, true, true> PersonSerializer;

void read_and_show_input(std::istream &infile)
{
    PersonRecord person;

    /* Make the ifstream readable to structstream */
    IOIntfHandle io(new StandardInputStream(infile));

    /* Parse the input */
    try {
        FromBitstream(
            io,
            RegistryHandle(new Registry()),
            deserialize<PersonSerializer>(person)
            ).read_all();
    } catch (FormatError &e) {
        std::cerr << "Input file was invalid: Format violation" << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        return;
    } catch (UnsupportedInput &e) {
        std::cerr << "Input file was invalid: Unsupported input" << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        return;
    } catch (LimitError &e) {
        std::cerr << "Input file was invalid!" << std::endl;
        std::cerr << e.what() << std::endl << std::endl;
        return;
    }

    std::cout << "Look what I found:" << std::endl;
    std::cout << "Name: " << person.name << std::endl;
    std::cout << "Age: " << person.age << std::endl;
    std::cout << std::endl;
}

int main()
{
    PersonRecord person;

    load_all_hashes();

    {
        /* We check whether this program ran before and whether it
         * created an info file. */
        std::ifstream infile("person.ss");
        if (!infile.good()) {
            std::cerr << "No input data file ... " << std::endl;
        } else {
            read_and_show_input(infile);
        }
    }

    {
        /* Collect new input from the user */
        std::cout << "Yay! Let's create a new person record." << std::endl;
        std::cout << "Name: " << std::flush;

        char name_buf[128];
        memset(name_buf, 0, sizeof(name_buf));
        std::cin.getline(name_buf, sizeof(name_buf)-1);
        person.name = name_buf;

        std::cout << "Age: " << std::flush;
        std::cin >> person.age;
        std::cout << "Great! I'll write it to person.ss." << std::endl;
    }

    {
        /* Open the output file and store the collected input in it*/
        std::ofstream outfile("person.ss");

        /* Make the ofstream usable to structstream */
        IOIntfHandle io(new StandardOutputStream(outfile));

        /* There are two options here, one simple and one complex. The
         * simple is sufficient for most cases, but we'll do the
         * complex one here for demonstration purposes, as it allows
         * to set hash functions for containers.
         */

        StreamSink bitstream_sink;

#ifdef WITH_GNUTLS
        /* Write the node tree to the file: first get a bitstream
         * writer */
        ToBitstreamHashing *outstream = new ToBitstreamHashing(io);
        outstream->set_armor_default(true);
        outstream->set_hash_function(RT_CONTAINER, 0x01, HT_SHA256);

        bitstream_sink = StreamSink(outstream);
#else
        /* Hashing requires GnuTLS. If no GnuTLS can be used, we have
         * to skip hashing. */
        bitstream_sink = StreamSink(new ToBitstream(io));
#endif
        serialize_to_sink<PersonSerializer>(person, bitstream_sink);
        bitstream_sink->end_of_stream();

    }

    return 0;
}
