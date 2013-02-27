#include <iostream>
#include <fstream>
#include <cstdint>

#include "structstream/structstream.hpp"

using namespace StructStream;

/* We will be reading and writing this structure to a structstream. */
struct PersonRecord {
    std::string name;
    uint32_t age;

    void set_name(const std::string &new_name) {
        name = new_name;
    };

    const std::string &get_name() const {
        return name;
    };

    void set_age(const uint32_t new_age) {
        age = new_age;
    };

    uint32_t get_age() const {
        return age;
    };
};

/* Here we define a class which will handle deserialization of our
 * struct. */
typedef deserialize_block<
    0x01,
    PersonRecord,
    deserialize_string<0x02, UTF8Record, PersonRecord, &PersonRecord::set_name>,
    deserialize_custom<0x03, UInt32Record, PersonRecord, uint32_t, &PersonRecord::set_age>
    > PersonDeserializer;

/* And similarily, a serializer, which converts the struct in a
 * structstream node tree. */
typedef serialize_block<
    0x01,
    PersonRecord,
    serialize_string<0x02, UTF8Record, PersonRecord, &PersonRecord::get_name>,
    serialize_custom<0x03, UInt32Record, PersonRecord, uint32_t, &PersonRecord::get_age>
    > PersonSerializer;

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
            /* Make the ifstream readable to structstream */
            IOIntfHandle io(new StandardInputStream(infile));

            /* Parse the input */
            ContainerHandle root = bitstream_to_tree(io);

            /* Extract the first element from the document */
            Container *child = std::dynamic_pointer_cast<Container>(*root->children_begin()).get();

            if (child != nullptr) {
                /* If it (a) exists and (b) is actually a container,
                 * we can go on with deserializing it */
                PersonDeserializer::deserialize(child, &person);
                std::cout << "Look what I found:" << std::endl;
                std::cout << "Name: " << person.name << std::endl;
                std::cout << "Age: " << person.age << std::endl;
                std::cout << std::endl;

                if (child->get_hashed() != HT_INVALID) {
                    std::cout << "The data was checksum'd and validated successfully." << std::endl;
                }
            } else {
                /* Otherwise, we better stop. */
                std::cout << "Hmm, that input doesn't look ok to me. I better ignore it." << std::endl;
            }
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

        /* Create a node tree using the serializer */
        NodeHandle tree = PersonSerializer::serialize(&person);

        /* There are two options here, one simple and one complex. The
         * simple is sufficient for most cases, but we'll do the
         * complex one here for demonstration purposes, as it allows
         * to set hash functions for containers.
         *
         * For reference, the simple one would be just one line:
         *
         *     tree_to_bitstream({tree}, io);
         */

#ifdef WITH_GNUTLS
        /* Write the node tree to the file: first get a bitstream
         * writer */
        ToBitstreamHashing *outstream = new ToBitstreamHashing(io);
        outstream->set_armor_default(true);
        outstream->set_hash_function(RT_CONTAINER, 0x01, HT_SHA256);

        FromTree(StreamSink(outstream), {tree});
#else
        /* Hashing requires GnuTLS. If no GnuTLS can be used, we have
         * to skip hashing. */
        tree_to_bitstream({tree}, io);
#endif
    }

    return 0;
}
